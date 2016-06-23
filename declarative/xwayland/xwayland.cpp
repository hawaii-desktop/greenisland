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

#include <GreenIsland/QtWaylandCompositor/QWaylandClient>
#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/private/qwaylandsurface_p.h>

#include "xwayland.h"
#include "xwaylandmanager.h"
#include "xwaylandshellsurface.h"
#include "xwaylandserver.h"

Q_LOGGING_CATEGORY(XWAYLAND, "greenisland.xwayland")
Q_LOGGING_CATEGORY(XWAYLAND_TRACE, "greenisland.xwayland.trace")

XWayland::XWayland(QObject *parent)
    : QObject(parent)
    , m_compositor(Q_NULLPTR)
    , m_enabled(false)
    , m_initialized(false)
    , m_server(Q_NULLPTR)
    , m_manager(Q_NULLPTR)
{
}

XWayland::~XWayland()
{
    delete m_server;
}

QWaylandCompositor *XWayland::compositor() const
{
    return m_compositor;
}

void XWayland::setCompositor(QWaylandCompositor *compositor)
{
    if (m_compositor) {
        qCWarning(XWAYLAND, "Cannot move XWayland to another compositor");
        return;
    }

    m_compositor = compositor;
    Q_EMIT compositorChanged();

    connect(m_compositor, &QWaylandCompositor::surfaceCreated,
            this, &XWayland::handleSurfaceCreated);
}

bool XWayland::isEnabled() const
{
    return m_enabled;
}

void XWayland::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    Q_EMIT enabledChanged();

    if (m_enabled && !m_initialized)
        initialize();
}

void XWayland::initialize()
{
    // Check whether we have a compositor assigned
    if (!m_compositor) {
        qCWarning(XWAYLAND, "Please assign a compositor to XWayland");
        return;
    }

    // Do not continue unless enabled
    if (!m_enabled) {
        qCWarning(XWAYLAND, "XWayland is disabled");
        return;
    }

    // Instantiate the server
    m_server = new XWaylandServer(m_compositor, this);
    connect(m_server, &XWaylandServer::started,
            this, &XWayland::serverStarted);

    // Window manager
    m_manager = new XWaylandManager(m_server, this);
    connect(m_manager, &XWaylandManager::shellSurfaceAdded,
            this, &XWayland::handleShellSurfaceAdded);
    connect(m_manager, &XWaylandManager::shellSurfaceRemoved,
            this, &XWayland::shellSurfaceClosed);

    // FIXME: Start after 2,5s to avoid blocking the event loop,
    // it should probably be started once all QQuickViews
    // has loaded the scene
    QTimer::singleShot(2500, [&] {
        if (!m_server->setup())
            qCWarning(XWAYLAND) << "Failed to setup XWayland";
    });

    m_initialized = true;
}

void XWayland::serverStarted()
{
    // Start window management
    m_manager->start(m_server->wmFd());
}

void XWayland::handleSurfaceCreated(QWaylandSurface *surface)
{
    // We are only interested in surfaces from Xwayland
    QWaylandClient *client = surface->client();
    if (client && client->client() != m_server->client())
        return;

    Q_FOREACH (XWaylandShellSurface *window, m_manager->m_unpairedWindows) {
        quint32 id = wl_resource_get_id(QWaylandSurfacePrivate::get(surface)->resource()->handle);

        qCDebug(XWAYLAND, "Unpaired surface %d vs %d", window->surfaceId(), id);
        if (window->surfaceId() == id) {
            window->setSurfaceId(0);
            window->setSurface(surface);
            m_manager->m_unpairedWindows.removeOne(window);
            break;
        }
    }
}

void XWayland::handleShellSurfaceAdded(XWaylandShellSurface *shellSurface)
{
    connect(shellSurface, &XWaylandShellSurface::surfaceChanged, this, [this, shellSurface] {
        Q_EMIT shellSurfaceCreated(shellSurface);
    });
}

#include "moc_xwayland.cpp"
