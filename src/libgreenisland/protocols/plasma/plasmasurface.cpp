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
    , m_compositor(qobject_cast<Compositor *>(surface->compositor()))
    , m_shell(shell)
    , m_surface(surface)
    , m_role(ShellWindowView::NoneRole)
    , m_deleting(true)
{
    // Create a view for the first output
    Output *output = qobject_cast<Output *>(m_surface->compositor()->outputs().at(0));
    m_view = new ShellWindowView(m_surface, output);
    qDebug() << "New Plasma surface" << m_surface;

    // Map surface
    connect(m_surface, &QuickSurface::configure, [&](bool hasBuffer) {
        // Not until it has a role (unless it's an unmap)
        if (m_role == ShellWindowView::NoneRole && hasBuffer)
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

ShellWindowView::Role PlasmaSurface::role() const
{
    return m_role;
}

ShellWindowView *PlasmaSurface::view() const
{
    return m_view;
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

ShellWindowView::Role PlasmaSurface::wl2Role(uint32_t role)
{
    switch (role) {
    case role_splash:
        return ShellWindowView::SplashRole;
    case role_desktop:
        return ShellWindowView::DesktopRole;
    case role_dashboard:
        return ShellWindowView::DashboardRole;
    case role_panel:
        return ShellWindowView::PanelRole;
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

ShellWindowView::Flags PlasmaSurface::wl2Flags(uint32_t wlFlags)
{
    ShellWindowView::Flags flags = 0;

    if (wlFlags & flag_panel_always_visible)
        flags |= ShellWindowView::PanelAlwaysVisible;
    if (wlFlags & flag_panel_auto_hide)
        flags |= ShellWindowView::PanelAutoHide;
    if (wlFlags & flag_panel_windows_can_cover)
        flags |= ShellWindowView::PanelWindowsCanCover;
    if (wlFlags & flag_panel_windows_go_below)
        flags |= ShellWindowView::PanelWindowsGoBelow;

    return flags;
}

QString PlasmaSurface::role2String(const ShellWindowView::Role &role)
{
    switch (role) {
    case ShellWindowView::SplashRole:
        return QStringLiteral("Splash");
    case ShellWindowView::DesktopRole:
        return QStringLiteral("Desktop");
    case ShellWindowView::DashboardRole:
        return QStringLiteral("Dashboard");
    case ShellWindowView::PanelRole:
        return QStringLiteral("Panel");
    case ShellWindowView::OverlayRole:
        return QStringLiteral("Overlay");
    case ShellWindowView::NotificationRole:
        return QStringLiteral("Notification");
    case ShellWindowView::LockRole:
        return QStringLiteral("Lock");
    default:
        break;
    }

    return QStringLiteral("None");
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

    m_surface->setGlobalPosition(QPointF(x, y));
}

void PlasmaSurface::surface_set_role(Resource *resource,
                                     uint32_t wlRole)
{
    Q_UNUSED(resource);

    ShellWindowView::Role role = wl2Role(wlRole);

    // Some roles are exclusive
    switch (role) {
    case ShellWindowView::SplashRole:
    case ShellWindowView::DesktopRole:
    case ShellWindowView::DashboardRole:
    case ShellWindowView::LockRole:
        for (PlasmaSurface *s: m_shell->surfaces()) {
            if (s->role() == role && s->view()->output() == m_view->output()) {
                const QString msg = QStringLiteral("Surface already has role \"%1\"");
                const QString errMsg = msg.arg(role2String(role));
                qWarning("%s", qPrintable(errMsg));
                wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                                       "%s", qPrintable(errMsg));
                return;
            }
        }
        break;
    default:
        break;
    }

    // Set role
    m_role = role;
    m_view->setRole(m_role);

    // Show splash layer when a splash role is set
    if (m_role == ShellWindowView::SplashRole)
        Q_EMIT m_compositor->fadeOut();

    // Set position to 0,0 for some roles
    switch (m_role) {
    case ShellWindowView::DesktopRole:
    case ShellWindowView::DashboardRole:
    case ShellWindowView::LockRole:
        m_surface->setGlobalPosition(m_view->output()->mapToGlobal(QPointF(0, 0)));
        break;
    default:
        break;
    }
}

void PlasmaSurface::surface_set_flags(Resource *resource,
                                      uint32_t wlFlags)
{
    Q_UNUSED(resource);

    ShellWindowView::Flags flags = wl2Flags(wlFlags);
    m_view->setFlags(flags);
}

void PlasmaSurface::surface_set_screen_edge(Resource *resource,
                                            uint32_t screen_edge)
{
    Q_UNUSED(resource);
    Q_UNUSED(screen_edge);
}

}
