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

#include <QtCompositor/QWaylandCompositor>

#include "output.h"
#include "plasmasurface.h"
#include "shellwindowview.h"

namespace GreenIsland {

PlasmaSurface::PlasmaSurface(PlasmaShell *shell, QWaylandSurface *surface,
                             wl_client *client, uint32_t id)
    : QWaylandSurfaceInterface(surface)
    , QtWaylandServer::org_kde_plasma_surface(client, id)
    , m_shell(shell)
    , m_surface(surface)
{
    // Create a view for the first output
    QWaylandQuickSurface *quickSurface = static_cast<QWaylandQuickSurface *>(m_surface);
    Output *output = qobject_cast<Output *>(m_surface->compositor()->outputs().at(0));
    m_view = new ShellWindowView(quickSurface, output);

    // Map surface
    connect(m_surface, &QWaylandSurface::configure, [=](bool hasBuffer) {
        m_surface->setMapped(hasBuffer);
    });
}

bool PlasmaSurface::runOperation(QWaylandSurfaceOp *op)
{
    switch (op->type()) {
    case QWaylandSurfaceOp::Close:
        // TODO: destroy
        return true;
    default:
        break;
    }

    return false;
}

void PlasmaSurface::surface_destroy(Resource *resource)
{
}

void PlasmaSurface::surface_set_output(Resource *resource,
                                       wl_resource *outputResource)
{
    Q_UNUSED(resource);

    Output *output = qobject_cast<Output *>(Output::fromResource(outputResource));
    m_view->setOutput(output);
}

void PlasmaSurface::surface_set_position(Resource *resource,
                                         int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    m_view->setPosition(QPointF(x, y));
}

void PlasmaSurface::surface_set_role(Resource *resource,
                                     uint32_t role)
{
    Q_UNUSED(resource);

    // Set position according to the role
    switch (role) {
    case QtWaylandServer::org_kde_plasma_surface::role_desktop:
        m_view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_surface::role_dashboard:
        m_view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_surface::role_lock:
        m_view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_surface::role_notification:
        m_view->setPosition(QPointF(0, 0));
        break;
    case QtWaylandServer::org_kde_plasma_surface::role_overlay:
        m_view->setPosition(QPointF(0, 0));
        break;
    default:
        break;
    }
}

}
