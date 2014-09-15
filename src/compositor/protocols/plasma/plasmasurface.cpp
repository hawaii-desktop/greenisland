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

#include <QtCore/QDebug>

#include "compositor.h"
#include "output.h"
#include "plasmasurface.h"
#include "quicksurface.h"
#include "shellwindowview.h"

namespace GreenIsland {

PlasmaSurface::PlasmaSurface(PlasmaShell *shell, QuickSurface *surface,
                             wl_client *client, uint32_t id)
    : QWaylandSurfaceInterface(surface)
    , QtWaylandServer::org_kde_plasma_surface(client, id)
    , m_shell(shell)
    , m_surface(surface)
    , m_role(role_none)
    , m_deleting(true)
{
    // Create a view for the first output
    Output *output = qobject_cast<Output *>(m_surface->compositor()->outputs().at(0));
    m_view = new ShellWindowView(m_surface, output);
    qDebug() << "New Plasma surface" << m_surface;

    // Map surface
    connect(m_surface, &QuickSurface::configure, [&](bool hasBuffer) {
        // Not until it has a role (unless it's an unmap)
        if (m_role == role_none && hasBuffer)
            return;
        m_surface->setMapped(hasBuffer);
    });
}

PlasmaSurface::~PlasmaSurface()
{
    // Don't destroy the resource if the destructor is called
    // from surface_destroy_resource()
    if (!m_deleting) {
        m_deleting = true;
        wl_resource_destroy(resource()->handle);
    }
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

ShellWindowView::Role PlasmaSurface::wl2Role(const role &role)
{
    switch (role) {
    case role_splash:
        return ShellWindowView::SplashRole;
    case role_desktop:
        return ShellWindowView::DesktopRole;
    case role_dashboard:
        return ShellWindowView::DashboardRole;
    case role_config:
        return ShellWindowView::PanelConfigRole;
    case role_overlay:
        return ShellWindowView::OverlayRole;
    case role_notification:
        return ShellWindowView::NotificationRole;
    case role_lock:
        return ShellWindowView::LockRole;
    default:
        break;
    }

    return ShellWindowView::NoneRole;
}

void PlasmaSurface::surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);

    // We might be called from the destructor, in that case avoid
    // deleting twice
    if (!m_deleting) {
        m_deleting = true;
        delete this;
    }
}

void PlasmaSurface::surface_destroy(Resource *resource)
{
    // Unmap surface
    m_surface->setMapped(false);

    // Destroy this object
    surface_destroy_resource(resource);
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

    // Set role
    m_role = static_cast<PlasmaSurface::role>(role);
    m_view->setRole(wl2Role(m_role));

    // Set position according to the role
    switch (m_role) {
    case role_desktop:
        m_surface->setGlobalPosition(QPointF(0, 0));
        break;
    case role_dashboard:
        m_surface->setGlobalPosition(QPointF(0, 0));
        break;
    case role_lock:
        m_surface->setGlobalPosition(QPointF(0, 0));
        break;
    case role_notification:
        m_surface->setGlobalPosition(QPointF(0, 0));
        break;
    case role_overlay:
        m_surface->setGlobalPosition(QPointF(0, 0));
        break;
    default:
        break;
    }
}

}
