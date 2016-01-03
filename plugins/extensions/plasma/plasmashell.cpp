/****************************************************************************
 * This file is part of Hawaii.
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

#include <GreenIsland/Compositor/QWaylandSurface>
#include <GreenIsland/Compositor/QWaylandSurfaceItem>
#include <waylandcompositor/wayland_wrapper/qwlinputdevice_p.h>
#include <waylandcompositor/wayland_wrapper/qwloutput_p.h>
#include <waylandcompositor/wayland_wrapper/qwlsurface_p.h>

#include "compositor.h"
#include "plasmashell.h"
#include "plasmasurface.h"

Q_LOGGING_CATEGORY(PLASMA_SHELL_PROTOCOL, "greenisland.plugins.plasma.shell")

namespace GreenIsland {

PlasmaShell::PlasmaShell(Compositor *compositor)
    : m_compositor(compositor)
{
}

PlasmaShell::~PlasmaShell()
{
    qDeleteAll(m_surfaces);
}

const wl_interface *PlasmaShell::interface() const
{
    return &org_kde_plasma_shell_interface;
}

void PlasmaShell::bind(wl_client *client, uint32_t version, uint32_t id)
{
    add(client, id, version);
}

QList<PlasmaSurface *> PlasmaShell::surfaces() const
{
    return m_surfaces;
}

void PlasmaShell::shell_get_surface(Resource *resource, uint32_t id,
                       wl_resource *surfaceResource)
{
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qCWarning(PLASMA_SHELL_PROTOCOL) << "Unable to retrieve surface from resource!";
        return;
    }

    PlasmaSurface *plasmaSurface = new PlasmaSurface(this, surface, resource->client(), id);
    m_surfaces.append(plasmaSurface);
    connect(surface, &QWaylandSurface::surfaceDestroyed, [=] {
      m_surfaces.removeOne(plasmaSurface);
      plasmaSurface->deleteLater();
    });
}

void PlasmaShell::shell_desktop_ready(Resource *resource)
{
    Q_UNUSED(resource);

    Q_EMIT m_compositor->fadeIn();
}

}

#include "moc_plasmashell.cpp"
