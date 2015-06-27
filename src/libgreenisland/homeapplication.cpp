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
#include "homeapplication.h"
#include "logging.h"
#include "utilities.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

namespace GreenIsland {

HomeApplication::HomeApplication()
    : m_idleTime(5000)
    , m_notify(false)
{
}

HomeApplication::~HomeApplication()
{
}

QString HomeApplication::shellName() const
{
    return GreenIsland::Compositor::s_fixedShell;
}

QString HomeApplication::socket() const
{
    return m_socket;
}

void HomeApplication::setSocket(const QString &socket)
{
    m_socket = socket;

    // Set environment so the shell can pick it up and reuse it
    // to start applications, this is needed when the compositor
    // is embedded into a full screen shell; in such cases the
    // default Wayland socket will be the one from full screen shell
    setenv("GREENISLAND_SOCKET", qPrintable(socket), 1);
}

QString HomeApplication::fakeScreenData() const
{
    return m_fakeScreenFileName;
}

void HomeApplication::setFakeScreenData(const QString &fileName)
{
    m_fakeScreenFileName = fileName;
}

int HomeApplication::idleTime() const
{
    return m_idleTime;
}

void HomeApplication::setIdleTime(int time)
{
    m_idleTime = time;
}

bool HomeApplication::notifyLoginManager() const
{
    return m_notify;
}

void HomeApplication::setNotifyLoginManager(bool notify)
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

    // Set nested mode and shell
    GreenIsland::Compositor::s_nested = nested;
    GreenIsland::Compositor::s_fixedShell = shell;

    // Check whether XDG_RUNTIME_DIR is ok or not
    GreenIsland::verifyXdgRuntimeDir();

    // If a socket is passed it means that we are nesting into
    // another compositor, let's do some checks
    if (!m_socket.isEmpty()) {
        // We need wayland QPA plugin
        if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            qCWarning(GREENISLAND_COMPOSITOR)
                    << "By passing the \"--socket\" argument you are requesting to nest"
                    << "this compositor into another, but you forgot to pass "
                    << "also \"-platform wayland\"!";
#if HAVE_SYSTEMD
            if (m_notify)
                sd_notifyf(0, "STATUS=Nesting requested, but no wayland QPA");
#endif
            return false;
        }
    }

    // Screen configuration
    if (!m_fakeScreenFileName.isEmpty()) {
        // Need the native backend
        if (QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            qCWarning(GREENISLAND_COMPOSITOR)
                    << "Fake screen configuration is not allowed when Green Island"
                    << "is nested into another compositor";
#if HAVE_SYSTEMD
            if (m_notify)
                sd_notifyf(0, "STATUS=Fake screen configuration not allowed when nested");
#endif
            return false;
        }
    }

    // Create the compositor
    Compositor *compositor = Compositor::instance();
    if (!m_fakeScreenFileName.isEmpty())
        compositor->setFakeScreenConfiguration(m_fakeScreenFileName);
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

} // namespace GreenIsland
