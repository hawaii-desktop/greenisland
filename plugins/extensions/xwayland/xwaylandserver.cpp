/****************************************************************************
 * This file is part of Hawaii.
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

/*
 * This code takes inspiration from the xwayland module from Weston,
 * which is:
 *
 * Copyright (C) 2011 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>
#include <QtGui/private/qguiapplication_p.h>

#include "xwayland.h"
#include "xwaylandserver.h"
#include "sigwatch.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <unistd.h>

#include <wayland-server.h>

namespace GreenIsland {

class ServerProcess : public QProcess
{
public:
    ServerProcess(QObject *parent = 0)
        : QProcess(parent)
    {
    }

protected:
    void setupChildProcess() Q_DECL_OVERRIDE
    {
        qCDebug(XWAYLAND) << "Setting up Xwayland process";

        // Ignore SIGUSR1 in the child, this will make Xwayland send
        // it to the parent (Green Island) when it's done initializing.
        // During the initialization, the X server will round trip and
        // block the Wayland compositor, so avoid making blocking requests
        // (like xcb_connect_to_fd) until it's done with that
        ::signal(SIGUSR1, SIG_IGN);
    }
};


XWaylandServer::XWaylandServer(Compositor *compositor, QObject *parent)
    : QObject(parent)
    , m_compositor(compositor)
    , m_display(0)
    , m_abstractFd(-1)
    , m_unixFd(-1)
    , m_wmFd(-1)
    , m_pid(-1)
    , m_process(Q_NULLPTR)
    , m_client(Q_NULLPTR)
{
}

XWaylandServer::~XWaylandServer()
{
    shutdown();
}

bool XWaylandServer::setup()
{
    int display = 0;
    QString lockFileName;

    while (true) {
        int status;
        lockFileName = createLockFile(display, status);
        if (lockFileName.isEmpty()) {
            if (status == EAGAIN) {
                continue;
            } else if (status == EEXIST) {
                display++;
                continue;
            } else {
                return false;
            }
        }

        m_abstractFd = bindToAbstractSocket(display);
        if (m_abstractFd < 0 && errno == EADDRINUSE) {
            display++;
            ::unlink(qPrintable(lockFileName));
            continue;
        }

        m_unixFd = bindToUnixSocket(display);
        if (m_unixFd < 0) {
            ::unlink(qPrintable(lockFileName));
            ::close(m_abstractFd);
            return false;
        }

        break;
    }

    m_displayName = QStringLiteral(":%1").arg(display);
    qCDebug(XWAYLAND, "XWayland server listening on display %s",
            qPrintable(m_displayName));
    qputenv("DISPLAY", qPrintable(m_displayName));

    UnixSignalWatcher *sigusr1 = new UnixSignalWatcher(this);
    connect(sigusr1, &UnixSignalWatcher::unixSignal, this, [this](int) {
        qCDebug(XWAYLAND) << "XWayland server ready";
        Q_EMIT started();
    });
    sigusr1->watchForSignal(SIGUSR1);

    return true;
}

QString XWaylandServer::createLockFile(int display, int &status)
{
    status = 0;

    const QString fileName = QStringLiteral("/tmp/.X%1-lock").arg(display);

    QFile lockFile(fileName);

    if (lockFile.open(QIODevice::WriteOnly))
        lockFile.setPermissions(QFileDevice::ReadUser |
                                QFileDevice::ReadGroup |
                                QFileDevice::ReadOther);
    else if (lockFile.exists()) {
        // Lock file already exists, open in read-only mode
        if (!lockFile.open(QIODevice::ReadOnly) || lockFile.read(11).size() != 11) {
            qCWarning(XWAYLAND, "Couldn't read lock file %s: %s",
                      qPrintable(fileName), qPrintable(lockFile.errorString()));

            if (lockFile.isOpen())
                lockFile.close();

            status = EEXIST;
            return QString();
        }

        // Read the PID of an existing Xorg server
        bool converted;
        lockFile.seek(0);
        pid_t pid = lockFile.readAll().trimmed().toInt(&converted);
        if (!converted) {
            qCWarning(XWAYLAND) << "Couldn't parse existing lock file" << fileName;
            lockFile.close();
            status = EEXIST;
            return QString();
        }

        // Check whether it is still running
        if (::kill(pid, 0) < 0 && errno == ESRCH) {
            // The lock file is stale: unlink and try again
            qCWarning(XWAYLAND) << "Deleting stale lock file" << fileName;
            if (lockFile.remove())
                status = EEXIST;
            else
                status = EAGAIN;
            lockFile.close();
            return QString();
        }

        lockFile.close();
        status = EEXIST;
        return QString();
    } else if (!lockFile.isOpen()) {
        qCWarning(XWAYLAND, "Failed to create lock file %s: %s",
                  qPrintable(fileName), qPrintable(lockFile.errorString()));
        return QString();
    }

    // If we are here we managed to create the lock file, now write the
    // pid of the Wayland compositor (rather than the Xwayland server)
    QString pidString;
    pidString.sprintf("%10lld\n", QCoreApplication::applicationPid());
    if (lockFile.write(pidString.toLatin1()) < 0) {
        qCWarning(XWAYLAND, "Failed to write to lock file %s: %s",
                  qPrintable(fileName), qPrintable(lockFile.errorString()));
        lockFile.remove();
        lockFile.close();
        return QString();
    }

    lockFile.close();
    return fileName;
}

int XWaylandServer::bindToAbstractSocket(int display)
{
    int fd = ::socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        qCWarning(XWAYLAND, "Failed to create abstract socket: %s",
                  ::strerror(errno));
        return -1;
    }

    sockaddr_un addr;
    addr.sun_family = AF_LOCAL;

    socklen_t nameSize = ::snprintf(addr.sun_path, sizeof addr.sun_path,
                                    "%c/tmp/.X11-unix/X%d", 0, display);
    socklen_t size = offsetof(sockaddr_un, sun_path) + nameSize;

    if (::bind(fd, (const sockaddr *)&addr, size) < 0) {
        qCWarning(XWAYLAND, "Failed to bind to @%s: %s", addr.sun_path + 1,
                  ::strerror(errno));
        ::close(fd);
        return -1;
    }

    qCDebug(XWAYLAND, "Created abstract socket @%s", addr.sun_path + 1);

    if (::listen(fd, 1) < 0) {
        ::close(fd);
        return -1;
    }

    // XWayland socket activation
    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(processEvents()));
    notifier->setEnabled(true);

    return fd;
}

int XWaylandServer::bindToUnixSocket(int display)
{
    int fd = ::socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        qCWarning(XWAYLAND, "Failed to create unix socket: %s",
                  ::strerror(errno));
        return -1;
    }

    sockaddr_un addr;
    addr.sun_family = AF_LOCAL;

    socklen_t nameSize = ::snprintf(addr.sun_path, sizeof addr.sun_path,
                                    "/tmp/.X11-unix/X%d", display) + 1;
    socklen_t size = offsetof(sockaddr_un, sun_path) + nameSize;

    ::unlink(addr.sun_path);

    if (::bind(fd, (const sockaddr *)&addr, size) < 0) {
        qCWarning(XWAYLAND, "Failed to bind to %s: %s", addr.sun_path,
                  ::strerror(errno));
        ::close(fd);
        return -1;
    }

    qCDebug(XWAYLAND, "Created unix socket %s", addr.sun_path);

    if (::listen(fd, 1) < 0) {
        ::unlink(addr.sun_path);
        ::close(fd);
        return -1;
    }

    // XWayland socket activation
    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(processEvents()));
    notifier->setEnabled(true);

    return fd;
}

pid_t XWaylandServer::spawn()
{
    int server[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, server) < 0) {
        qCWarning(XWAYLAND, "Failed to create socket pair for XWayland server: %s",
                  strerror(errno));
        return -1;
    }

    int wm[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, wm) < 0) {
        qCWarning(XWAYLAND, "Failed to create socket pair for window manager: %s",
                  strerror(errno));
        return -1;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("WAYLAND_SOCKET"), QString::number(::dup(server[1])));

    m_process = new ServerProcess();
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->setProcessEnvironment(env);
    connect(m_process, (void (QProcess::*)())&QProcess::started, [this, server, wm] {
        qCDebug(XWAYLAND) << "Xwayland started";

        ::close(server[1]);
        m_client = wl_client_create(m_compositor->waylandDisplay(), server[0]);

        ::close(wm[1]);
        m_wmFd = wm[0];
    });
    connect(m_process, (void (QProcess::*)(int))&QProcess::finished, [this](int exitCode) {
        qCDebug(XWAYLAND) << "Xwayland finished with exit code" << exitCode;

        if (m_process) {
            delete m_process;
            m_process = Q_NULLPTR;
        }
    });

    QString abstractFdString = QString::number(::dup(m_abstractFd));
    QString unixFdString = QString::number(::dup(m_unixFd));
    QString wmFdString = QString::number(::dup(wm[1]));
    QStringList args = QStringList()
            << m_displayName
            << QStringLiteral("-rootless")
            << QStringLiteral("-listen") << abstractFdString
            << QStringLiteral("-listen") << unixFdString
            << QStringLiteral("-wm") << wmFdString
            << QStringLiteral("-terminate");
    qCDebug(XWAYLAND) << "Running:" << "Xwayland" << qPrintable(args.join(QStringLiteral(" ")));
    m_process->start(QStringLiteral("Xwayland"), args);

    return m_process->processId();
}

void XWaylandServer::shutdown()
{
    // Terminate XWayland server
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished();
        delete m_process;
    }

    ::unlink(qPrintable(m_lockFileName));
}

void XWaylandServer::processEvents()
{
    if (!m_process) {
        m_pid = spawn();
        if (m_pid == -1) {
            qCWarning(XWAYLAND) << "Failed to spawn XWayland server";
        } else {
            Q_EMIT spawned();
            qCDebug(XWAYLAND) << "XWayland server pid" << m_pid;
        }
    }

    QSocketNotifier *notifier = qobject_cast<QSocketNotifier *>(sender());
    if (notifier) {
        disconnect(notifier, SIGNAL(activated(int)), this, SLOT(processEvents()));
        notifier->deleteLater();
    }
}

}
