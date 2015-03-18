/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCompositor/QtCompositorVersion>

#include "wlshell.h"
#include "wlshellsurface.h"
#include "wlshellsurfacepopupgrabber.h"

Q_LOGGING_CATEGORY(WLSHELL_PROTOCOL, "greenisland.protocols.wlshell")

namespace GreenIsland {

WlShell::WlShell()
{
}

const wl_interface *WlShell::interface() const
{
    return &wl_shell_interface;
}

void WlShell::bind(wl_client *client, uint32_t version, uint32_t id)
{
#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    add(client, id, version);
#else
    add(client, id);
#endif
}

WlShellSurfacePopupGrabber *WlShell::popupGrabberForDevice(QtWayland::InputDevice *device)
{
    // Create popup grabbers on demand
    if (!m_popupGrabbers.contains(device))
        m_popupGrabbers.insert(device, new WlShellSurfacePopupGrabber(device));
    return m_popupGrabbers.value(device);
}

void WlShell::shell_get_shell_surface(Resource *resource, uint32_t id,
                                      wl_resource *surfaceResource)
{
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    WlShellSurface *shellSurface = new WlShellSurface(this, surface, resource->client(), id);
    QObject::connect(surface, &QWaylandSurface::surfaceDestroyed, [=] {
        shellSurface->deleteLater();
    });
}

}
