/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/private/qobject_p.h>

#include "logging.h"
#include "logind/logind.h"
#include "logind/vthandler.h"

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <sys/ioctl.h>
#include <linux/major.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
}

#define RELEASE_SIGNAL      SIGUSR1 //SIGRTMIN
#define ACQUISITION_SIGNAL  SIGUSR2 //SIGRTMIN + 1

#ifndef KDSKBMUTE
#define KDSKBMUTE 0x4B51
#endif

namespace GreenIsland {

namespace Platform {

/*
 * VtHandlerPrivate
 */

class VtHandlerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(VtHandler)
public:
    VtHandlerPrivate(VtHandler *self)
        : logind(Logind::instance())
        , signalFd(-1)
        , notifier(Q_NULLPTR)
        , vtFd(-1)
        , vtNumber(0)
        , kbMode(K_OFF)
        , active(false)
        , q_ptr(self)
    {
        toggleTtyCursor(false);
    }

    void setup(int nr)
    {
        Q_Q(VtHandler);

        // A fd already open means we are good
        if (vtFd != -1)
            return;

        // Need a valid vt number
        if (nr < 0) {
            qCWarning(lcLogind, "Invalid vt number");
            return;
        }

        // Open the tty for this vt
        const QString devName = QStringLiteral("/dev/tty%1").arg(nr);
        vtFd = ::open(qPrintable(devName), O_RDWR | O_CLOEXEC | O_NONBLOCK);
        if (vtFd < 0) {
            qCWarning(lcLogind, "Failed to open vt \"%s\": %s",
                      qPrintable(devName), qPrintable(::strerror(errno)));
            return;
        }

        // Must be a valid vt
        if (isValidVt(vtFd)) {
            qCWarning(lcLogind, "TTY \"%s\" is not a virtual terminal",
                      qPrintable(devName));
            closeFd();
            return;
        }

        // Read keyboard mode
        if (::ioctl(vtFd, KDGKBMODE, &kbMode) < 0) {
            qCWarning(lcLogind, "Unable to read keyboard mode on \"%s\": %s",
                      qPrintable(devName), qPrintable(::strerror(errno)));
            kbMode = K_UNICODE;
        } else if (kbMode == K_OFF) {
            kbMode = K_UNICODE;
        }

        // Avoid input going to the tty
        if (::ioctl(vtFd, KDSKBMUTE, 1) < 0 &&
                ::ioctl(vtFd, KDSKBMODE, K_OFF) < 0) {
            qCWarning(lcLogind, "Unable to set K_OFF keyboard mode on \"%s\": %s",
                      qPrintable(devName), qPrintable(::strerror(errno)));
            closeFd();
        }

        // Graphics mode
        if (::ioctl(vtFd, KDSETMODE, KD_GRAPHICS) < 0) {
            qCWarning(lcLogind, "Unable to set KD_GRAPHICS mode on \"%s\": %s",
                      qPrintable(devName), qPrintable(::strerror(errno)));
            restoreKeyboard();
            closeFd();
            return;
        }

        // Signal handler
        if (!installSignalHandler()) {
            ::ioctl(vtFd, KDSETMODE, KD_TEXT);
            restoreKeyboard();
            closeFd();
            return;
        }

        // Take over VT
        vt_mode mode = { VT_PROCESS, 0, short(SIGRTMIN), short(ACQUISITION_SIGNAL), 0 };
        if (::ioctl(vtFd, VT_SETMODE, &mode) < 0) {
            qCWarning(lcLogind, "Unable to take over VT \"%s\": %s",
                      qPrintable(devName), qPrintable(::strerror(errno)));
            ::ioctl(vtFd, KDSETMODE, KD_TEXT);
            restoreKeyboard();
            closeFd();
            return ;
        }

        vtNumber = nr;
        setActive(true);
        Q_EMIT q->created();
    }

    bool installSignalHandler()
    {
        Q_Q(VtHandler);

        // SIGRTMIN is used as global VT-release signal while SIGRTMIN + 1 is
        // used as VT-acquire signal, these must be checked on runtime because
        // their exact values are unknown at compile time; verify if we
        // exceed the limit (POSIX has 32 of them)
        if (ACQUISITION_SIGNAL > SIGRTMAX) {
            qCWarning(lcLogind, "Not enough RT signals available: %u-%u",
                      SIGRTMIN, SIGRTMAX);
            return false;
        }

        // Block other signals and use signalfd
        sigset_t mask;
        ::sigemptyset(&mask);

        // Catch Ctlr+C
        ::sigaddset(&mask, SIGINT);

        // Catch Ctrl+Z (must handle suspend from the device integration)
        ::sigaddset(&mask, SIGTSTP);
        ::sigaddset(&mask, SIGCONT);

        // KIll
        ::sigaddset(&mask, SIGTERM);

        // Release and acquire
        ::sigaddset(&mask, RELEASE_SIGNAL);
        ::sigaddset(&mask, ACQUISITION_SIGNAL);

        signalFd = ::signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
        if (signalFd < 0) {
            qCWarning(lcLogind, "Failed to create signalfd: %d",
                      qPrintable(::strerror(errno)));
            ::ioctl(vtFd, KDSETMODE, KD_TEXT);
            restoreKeyboard();
            closeFd();
            return false;
        }

        notifier = new QSocketNotifier(signalFd, QSocketNotifier::Read, q);
        q->connect(notifier, &QSocketNotifier::activated, q, [this, q] {
            if (vtFd < 0)
                return;

            notifier->setEnabled(false);

            signalfd_siginfo sig;
            if (::read(signalFd, &sig, sizeof(sig)) == sizeof(sig)) {
                switch (sig.ssi_signo) {
                case SIGINT:
                case SIGTERM:
                    Q_EMIT q->interrupted();
                    restoreKeyboard();
                    toggleTtyCursor(true);
                    ::_exit(1);
                    break;
                case SIGTSTP:
                    Q_EMIT q->suspendRequested();
                    break;
                case SIGCONT:
                    Q_EMIT q->resumed();
                    break;
                case RELEASE_SIGNAL:
                    setActive(false);
                    ::ioctl(vtFd, VT_RELDISP, 1);
                    break;
                case ACQUISITION_SIGNAL:
                    ::ioctl(vtFd, VT_RELDISP, VT_ACKACQ);
                    setActive(true);
                    break;
                default:
                    break;
                }
            }

            notifier->setEnabled(true);
        });

        // Block signals that are handled via signalfd, this is done only for
        // the current thread, but new threads will inherit the signal mask
        // from the creator
        ::pthread_sigmask(SIG_BLOCK, &mask, Q_NULLPTR);

        return true;
    }

    void toggleTtyCursor(bool enable)
    {
        int fd = -1;

        const char *const devices[] = { "/dev/tty0", "/dev/tty", "/dev/console", Q_NULLPTR };
        for (const char *const *device = devices; *device; ++device) {
            fd = ::open(*device, O_RDWR);
            if (fd >= 0) {
                const char *escape = enable
                        ? "\033[9;15]\033[?33h\033[?25h\033[?0c"
                        : "\033[9;0]\033[?33l\033[?25l\033[?1c";
                ::write(fd, escape, ::strlen(escape) + 1);
                ::close(fd);
                return;
            }
        }
    }

    void restoreKeyboard()
    {
        if (vtFd < 0)
            return;

        ::ioctl(vtFd, KDSKBMUTE, 0);
        ::ioctl(vtFd, KDSKBMUTE, kbMode);
    }

    void closeFd()
    {
        if (vtFd < 0)
            return;

        if (notifier) {
            delete notifier;
            notifier = Q_NULLPTR;

            ::close(signalFd);
            signalFd = -1;
        }

        ::close(vtFd);
        vtFd = -1;
    }

    void setActive(bool v)
    {
        Q_Q(VtHandler);

        if (active == v)
            return;

        active = v;
        Q_EMIT q->activeChanged(v);
    }

    static bool isValidVt(int fd)
    {
        if (fd < 0)
            return false;

        struct stat st;
        if (::fstat(fd, &st) == -1) {
            qCWarning(lcLogind) << "Failed to fstat tty";
            return false;
        }

        if (major(st.st_rdev) != TTY_MAJOR || minor(st.st_rdev) <= 0 ||
                minor(st.st_rdev) >= 64)
            return false;

        return true;
    }

    Logind *logind;

    int signalFd;
    QSocketNotifier *notifier;

    int vtFd;
    int vtNumber;

    int kbMode;

    bool active;

private:
    VtHandler *q_ptr;
};

/*
 * VtHandler
 */

VtHandler::VtHandler(QObject *parent)
    : QObject(*new VtHandlerPrivate(this), parent)
{
    Q_D(VtHandler);

    // Setup vt if we already know the number
    if (d->logind->vtNumber() != -1)
        d->setup(d->logind->vtNumber());

    // Setup vt when the number will be assigned
    connect(d->logind, &Logind::vtNumberChanged, this, [this, d](int nr) {
        d->setup(nr);
    });

    // Take control upon connection
    if (d->logind->isConnected())
        d->logind->takeControl();
    else
        connect(d->logind, &Logind::connectedChanged,
                d->logind, &Logind::takeControl);
}

VtHandler::~VtHandler()
{
    Q_D(VtHandler);

    d->restoreKeyboard();
    d->toggleTtyCursor(true);
    if (d->signalFd != -1)
        ::close(d->signalFd);
    d->closeFd();
}

bool VtHandler::isActive() const
{
    Q_D(const VtHandler);
    return d->active;
}

void VtHandler::activate(int nr)
{
    Q_D(VtHandler);

    if (d->vtFd < 0)
        return;

    if (d->vtNumber == nr)
        return;

    if (::ioctl(d->vtFd, VT_ACTIVATE, nr) < 0) {
        qCWarning(lcLogind, "Unable to activate vt number %d: %s",
                  nr, qPrintable(::strerror(errno)));
        return;
    }

    d->setActive(false);
}

void VtHandler::suspend()
{
    ::kill((pid_t)QCoreApplication::applicationPid(), SIGSTOP);
}

} // namespace Platform

} // namespace GreenIsland
