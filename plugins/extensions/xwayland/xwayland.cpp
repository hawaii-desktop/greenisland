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

#include <QtCore/QTimer>
#include <GreenIsland/Compositor/QWaylandClient>
#include <waylandcompositor/wayland_wrapper/qwlsurface_p.h>

#include "compositor.h"
#include "xwayland.h"
#include "xwaylandmanager.h"
#include "xwaylandwindow.h"
#include "xwaylandserver.h"

Q_LOGGING_CATEGORY(XWAYLAND, "greenisland.xwayland")
Q_LOGGING_CATEGORY(XWAYLAND_TRACE, "greenisland.xwayland.trace")

namespace GreenIsland {

XWayland::XWayland(Compositor *compositor, QObject *parent)
    : QObject(parent)
    , m_compositor(compositor)
{
    // Server
    m_server = new XWaylandServer(m_compositor, this);
    connect(m_server, &XWaylandServer::started,
            this, &XWayland::serverStarted);

    // Window manager
    m_manager = new XWaylandManager(m_compositor, m_server, this);
}

XWayland::~XWayland()
{
}

bool XWayland::isInitialized() const
{
    return m_server && m_manager;
}

void XWayland::initialize()
{
    // FIXME: Start after 2,5s to avoid blocking the event loop,
    // it should probably be started once all QQuickViews
    // has loaded the scene
    QTimer::singleShot(2500, [&] {
        if (!m_server->setup())
            qCWarning(XWAYLAND) << "Failed to setup XWayland";
    });
}

void XWayland::serverStarted()
{
    // Listen for compositor events
    connect(m_compositor, &Compositor::newSurfaceCreated,
            this, &XWayland::surfaceCreated,
            Qt::QueuedConnection);

    // Start window management
    m_manager->start(m_server->wmFd());
}

void XWayland::surfaceCreated(QWaylandSurface *surface)
{
    // We are only interested in surfaces from Xwayland
    QWaylandClient *client = surface->client();
    if (client && client->client() != m_server->client())
        return;

    Q_FOREACH (XWaylandWindow *window, m_manager->m_unpairedWindows) {
        int id = wl_resource_get_id(surface->handle()->resource()->handle);

        qCDebug(XWAYLAND, "Unpaired surface %d vs %d", window->surfaceId(), id);
        if (window->surfaceId() == id) {
            window->setSurface(surface);
            window->setSurfaceId(0);
            m_manager->m_unpairedWindows.removeOne(window);
            break;
        }
    }
}

}

#include "moc_xwayland.cpp"
