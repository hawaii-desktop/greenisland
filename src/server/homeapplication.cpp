/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtGui/QGuiApplication>

#include "config.h"
#include "compositor.h"
#include "compositor_p.h"
#include "diagnostic_p.h"
#include "homeapplication.h"
#include "logging.h"
#include "utilities.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#ifndef KDSKBMUTE
#define KDSKBMUTE 0x4B51
#endif

#ifdef K_OFF
#define KBD_OFF_MODE K_OFF
#else
#define KBD_OFF_MODE K_RAW
#endif

namespace GreenIsland {

HomeApplication::HomeApplication()
    : m_idleInterval(0)
    , m_notify(false)
    , m_tty(-1)
    , m_oldKbdMode(-1)
{
    // Diagnostic output
    std::wstring output;
    output += DiagnosticOutput::systemInformation().toStdWString();
    //output += DiagnosticOutput::openGlContext().toStdWString();
    output += DiagnosticOutput::framework().toStdWString();
    output += DiagnosticOutput::environment().toStdWString();
    std::wcout << output;

    // Setup tty execept when running inside another
    // Wayland compositor or X11
    if (qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY") && qEnvironmentVariableIsEmpty("DISPLAY"))
        setupTty();
}

HomeApplication::~HomeApplication()
{
    restoreTty();
}

QString HomeApplication::fakeScreenData() const
{
    return m_fakeScreenFileName;
}

void HomeApplication::setFakeScreenData(const QString &fileName)
{
    m_fakeScreenFileName = fileName;
}

int HomeApplication::idleInterval() const
{
    return m_idleInterval;
}

void HomeApplication::setIdleInterval(int time)
{
    m_idleInterval = time;
}

bool HomeApplication::isNotificationEnabled() const
{
    return m_notify;
}

void HomeApplication::setNotificationEnabled(bool notify)
{
#if HAVE_SYSTEMD
    m_notify = notify;
#endif
}

bool HomeApplication::run(bool nested, const QString &shell)
{
    // If a compositor is already running we cannot continue
    if (Compositor::instance()->isRunning()) {
        qCWarning(GREENISLAND_COMPOSITOR) << "Compositor already running, don't call run() more than once!";
        return false;
    }

    // Check whether XDG_RUNTIME_DIR is ok or not
    GreenIsland::verifyXdgRuntimeDir();

    // Create the compositor
    Compositor *compositor = Compositor::instance();
    compositor->d_func()->nested = nested;
    compositor->d_func()->shell = shell;
    if (m_idleInterval > 0)
        compositor->d_func()->idleInterval = m_idleInterval;
    if (!m_fakeScreenFileName.isEmpty())
        compositor->d_func()->fakeScreenConfiguration = m_fakeScreenFileName;
    QObject::connect(compositor, &Compositor::screenConfigurationAcquired, [this] {
#if HAVE_SYSTEMD
        // Notify systemd when the screen configuration is ready
        if (m_notify) {
            qCDebug(GREENISLAND_COMPOSITOR) << "Compositor ready, notify systemd on" << qgetenv("NOTIFY_SOCKET");
            sd_notify(0, "READY=1");
        }
#endif
    });
    compositor->run();
    compositorLaunched();

    return true;
}

void HomeApplication::compositorLaunched()
{
}

void HomeApplication::setupTty()
{
    const QString vtNr = QString::fromUtf8(qgetenv("XDG_VTNR"));
    m_ttyString = QString("/dev/tty%1").arg(vtNr);

    m_tty = ::open(m_ttyString.toUtf8().constData(), O_RDWR | O_CLOEXEC | O_NONBLOCK);
    if (m_tty < 0) {
        qCWarning(GREENISLAND_COMPOSITOR, "Failed to open %s: %s",
                  qPrintable(m_ttyString), ::strerror(errno));
        return;
    }

    // Save current keyboard mode
    ioctl(m_tty, KDGKBMODE, &m_oldKbdMode);

    // Disable the tty keyboard
    ioctl(m_tty, KDSKBMUTE, 1);
    ioctl(m_tty, KDSKBMODE, KBD_OFF_MODE);

    // Graphics mode
    if (ioctl(m_tty, KDSETMODE, KD_GRAPHICS) < 0) {
        qCWarning(GREENISLAND_COMPOSITOR, "Failed to set %s in graphics mode: %s",
                  qPrintable(m_ttyString), ::strerror(errno));
        ::close(m_tty);
        m_tty = -1;
        return;
    }
}

void HomeApplication::restoreTty()
{
    if (m_tty == -1 || m_oldKbdMode == -1)
        return;

    ioctl(m_tty, KDSKBMUTE, 0);
    ioctl(m_tty, KDSKBMODE, m_oldKbdMode);

    if (ioctl(m_tty, KDSETMODE, KD_TEXT) < 0)
        qCWarning(GREENISLAND_COMPOSITOR, "Failed to set %s in text mode: %s",
                  qPrintable(m_ttyString), ::strerror(errno));

    ::close(m_tty);
    m_tty = -1;
}

} // namespace GreenIsland
