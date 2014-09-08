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

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceItem>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwloutput_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "output.h"
#include "plasmashell.h"
#include "windowview.h"

PlasmaShell::PlasmaShell()
{
}

const wl_interface *PlasmaShell::interface() const
{
    return &org_kde_plasma_shell_interface;
}

void PlasmaShell::bind(wl_client *client, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);

    add(client, id);
}

void PlasmaShell::shell_set_position(Resource *resource, wl_resource *outputResource,
                        wl_resource *surfaceResource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    // We have only one view for shell windows
    if (surface->views().size() != 1)
        return;

    // Move view to global coordinates
    QWaylandSurfaceItem *view = static_cast<QWaylandSurfaceItem *>(surface->views().at(0));

    Output *output = qobject_cast<Output *>(Output::fromResource(outputResource));
    view->setPosition(output->mapToGlobal(QPointF(x, y)));
}

void PlasmaShell::shell_set_surface_role(Resource *resource, wl_resource *output,
                            wl_resource *surfaceResource, uint32_t role)
{
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    // Role can be set only once
    if (surface->views().size() > 0)
        return;

    // Create a surface item
    QWaylandQuickSurface *quickSurface = qobject_cast<QWaylandQuickSurface *>(surface);
    if (!quickSurface) {
        qWarning() << "Surface" << surface << "doesn't inherit from QWaylandQuickSurface";
        return;
    }
    QWaylandSurfaceItem *view = new QWaylandSurfaceItem(quickSurface);

    // Set position according to the role
    switch (role) {
    case QtWaylandServer::org_kde_plasma_shell::role_desktop:
        view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_shell::role_dashboard:
        view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_shell::role_lock:
        view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_shell::role_notification:
        view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_shell::role_overlay:
        view->setPosition(QPointF(0, 0));
        break;
    default:
        break;
    }
}

void PlasmaShell::shell_desktop_ready(Resource *resource)
{
    Q_UNUSED(resource);
}
