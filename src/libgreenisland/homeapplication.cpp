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
#include "homeapplication.h"
#include "logging.h"
#include "utilities.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

namespace GreenIsland {

HomeApplication::HomeApplication()
    : m_idleInterval(0)
    , m_notify(false)
{
}

HomeApplication::~HomeApplication()
{
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

} // namespace GreenIsland
