/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "compositor.h"
#include "config.h"
#include "globalregistry.h"
#include "homeapplication.h"
#include "logging.h"
#include "utilities.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

namespace GreenIsland {

HomeApplication::HomeApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_idleTime(5000)
    , m_compositor(Q_NULLPTR)
{
    // Application
    setApplicationName("Green Island");
    setApplicationVersion(GREENISLAND_VERSION_STRING);
    setQuitOnLastWindowClosed(false);
}

HomeApplication::~HomeApplication()
{
    m_compositor->deleteLater();
}

Compositor *HomeApplication::compositor() const
{
    return m_compositor;
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

bool HomeApplication::run(const QString &shell)
{
    // If a compositor is already running we cannot continue
    if (m_compositor) {
        qCWarning(GREENISLAND_COMPOSITOR) << "Compositor already running, don't call run() more than once!";
        return false;
    }

    // Set plugin
    GreenIsland::Compositor::s_fixedShell = shell;

    // Check whether XDG_RUNTIME_DIR is ok or not
    GreenIsland::verifyXdgRuntimeDir();

    // If a socket is passed it means that we are nesting into
    // another compositor, let's do some checks
    if (!m_socket.isEmpty()) {
        // We need wayland QPA plugin
        if (!QApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            qCWarning(GREENISLAND_COMPOSITOR)
                    << "By passing the \"--socket\" argument you are requesting to nest"
                    << "this compositor into another, but you forgot to pass "
                    << "also \"-platform wayland\"!";
#if HAVE_SYSTEMD
            sd_notifyf(0, "STATUS=Nesting requested, but no wayland QPA");
#endif
            return false;
        }
    }

    // Screen configuration
    if (!m_fakeScreenFileName.isEmpty()) {
        // Need the native backend
        if (QApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            qCWarning(GREENISLAND_COMPOSITOR)
                    << "Fake screen configuration is not allowed when Green Island"
                    << "is nested into another compositor";
#if HAVE_SYSTEMD
            sd_notifyf(0, "STATUS=Fake screen configuration not allowed when nested");
#endif
            return false;
        }
    }

    // Bind to globals such as full screen shell if we are a Wayland client
    if (QApplication::platformName().startsWith(QStringLiteral("wayland")))
        GreenIsland::GlobalRegistry::instance()->start();

    // Create the compositor
    m_compositor = new GreenIsland::Compositor(m_socket);
    if (!m_fakeScreenFileName.isEmpty())
        m_compositor->setFakeScreenConfiguration(m_fakeScreenFileName);
    m_compositor->run();

    return true;
}

} // namespace GreenIsland

#include "moc_homeapplication.cpp"
