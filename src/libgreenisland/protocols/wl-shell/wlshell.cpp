/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCompositor/QtCompositorVersion>

#include "wlshell.h"
#include "wlshellsurface.h"
#include "wlshellsurfacepopupgrabber.h"

Q_LOGGING_CATEGORY(WLSHELL_PROTOCOL, "greenisland.protocols.wlshell")
Q_LOGGING_CATEGORY(WLSHELL_TRACE, "greenisland.protocols.wlshell.trace")

namespace GreenIsland {

WlShellGlobal::WlShellGlobal(QObject *parent)
    : QObject(parent)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;
}

const wl_interface *WlShellGlobal::interface() const
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    return &wl_shell_interface;
}

void WlShellGlobal::bind(wl_client *client, uint32_t version, uint32_t id)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    new WlShell(client, id, version, this);
}

WlShell::WlShell(wl_client *client, uint32_t name, uint32_t version, QObject *parent)
    : QObject(parent)
    , QtWaylandServer::wl_shell(client, name, version)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;
}

WlShell::~WlShell()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
}

void WlShell::shell_destroy_resource(Resource *resource)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    delete this;
}

void WlShell::shell_get_shell_surface(Resource *resource, uint32_t id,
                                      wl_resource *surfaceResource)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    new WlShellSurface(this, surface, resource->client(), id, resource->version());
}

WlShellSurfacePopupGrabber *WlShell::popupGrabberForDevice(QtWayland::InputDevice *device)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Create popup grabbers on demand
    if (!m_popupGrabbers.contains(device))
        m_popupGrabbers.insert(device, new WlShellSurfacePopupGrabber(device));
    return m_popupGrabbers.value(device);
}

}
