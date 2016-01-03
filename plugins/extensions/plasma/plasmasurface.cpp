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

#include <QtCore/QDebug>
#include <waylandcompositor/wayland_wrapper/qwlsurface_p.h>

#include "compositor.h"
#include "compositor_p.h"
#include "plasmasurface.h"
#include "output.h"

namespace GreenIsland {

PlasmaSurface::PlasmaSurface(PlasmaShell *shell, QWaylandSurface *surface,
                             wl_client *client, uint32_t id)
    : QWaylandSurfaceInterface(surface)
    , QtWaylandServer::org_kde_plasma_surface(client, id, 1)
    , m_compositor(static_cast<Compositor *>(surface->compositor()))
    , m_shell(shell)
    , m_surface(surface)
    , m_window(new ShellWindow(surface, this))
    , m_deleting(false)
{
    // Surface events
    connect(m_surface, &QWaylandSurface::configure, [&](bool hasBuffer) {
        // Map or unmap the surface
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

ShellWindow *PlasmaSurface::window() const
{
    return m_window;
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

ShellWindow::Role PlasmaSurface::wl2Role(uint32_t role)
{
    switch (role) {
    case role_splash:
        return ShellWindow::SplashRole;
    case role_desktop:
        return ShellWindow::DesktopRole;
    case role_dashboard:
        return ShellWindow::DashboardRole;
    case role_panel:
        return ShellWindow::PanelRole;
    case role_overlay:
        return ShellWindow::OverlayRole;
    case role_notification:
        return ShellWindow::NotificationRole;
    case role_lock:
        return ShellWindow::LockRole;
    default:
        break;
    }

    return ShellWindow::UnknownRole;
}

ShellWindow::Flags PlasmaSurface::wl2Flags(uint32_t wlFlags)
{
    ShellWindow::Flags flags = 0;

    if (wlFlags & flag_panel_always_visible)
        flags |= ShellWindow::PanelAlwaysVisible;
    if (wlFlags & flag_panel_auto_hide)
        flags |= ShellWindow::PanelAutoHide;
    if (wlFlags & flag_panel_windows_can_cover)
        flags |= ShellWindow::PanelWindowsCanCover;
    if (wlFlags & flag_panel_windows_go_below)
        flags |= ShellWindow::PanelWindowsGoBelow;

    return flags;
}

QString PlasmaSurface::role2String(const ShellWindow::Role &role)
{
    switch (role) {
    case ShellWindow::SplashRole:
        return QStringLiteral("Splash");
    case ShellWindow::DesktopRole:
        return QStringLiteral("Desktop");
    case ShellWindow::DashboardRole:
        return QStringLiteral("Dashboard");
    case ShellWindow::PanelRole:
        return QStringLiteral("Panel");
    case ShellWindow::OverlayRole:
        return QStringLiteral("Overlay");
    case ShellWindow::NotificationRole:
        return QStringLiteral("Notification");
    case ShellWindow::LockRole:
        return QStringLiteral("Lock");
    default:
        break;
    }

    return QStringLiteral("Unknown");
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

    // Move the surface to another output
    // TODO: Maybe check whether a surface with the same role already exist
    // on the new output
    QList<QtWayland::Output *> outputs = m_surface->handle()->outputs();
    m_surface->handle()->addToOutput(QWaylandOutput::fromResource(outputResource)->handle());
    Q_FOREACH (QtWayland::Output *output, outputs)
        m_surface->handle()->removeFromOutput(output);
    Q_EMIT m_window->outputChanged();
}

void PlasmaSurface::surface_set_position(Resource *resource,
                                         int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    Q_EMIT m_window->moveRequested(QPointF(x, y));
}

void PlasmaSurface::surface_set_role(Resource *resource,
                                     uint32_t wlRole)
{
    Q_UNUSED(resource);

    ShellWindow::Role role = wl2Role(wlRole);

    // Some roles are exclusive
    switch (role) {
    case ShellWindow::SplashRole:
    case ShellWindow::DesktopRole:
    case ShellWindow::DashboardRole:
    case ShellWindow::LockRole:
        Q_FOREACH (const PlasmaSurface *plasmaSurface, m_shell->surfaces()) {
            if (plasmaSurface->window() == m_window)
                continue;
            if (plasmaSurface->window()->role() == role) {
                const QString msg = QStringLiteral("Surface already has role \"%1\"");
                const QString errMsg = msg.arg(role2String(role));
                qCWarning(PLASMA_SHELL_PROTOCOL) << qPrintable(errMsg);
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
    m_window->setRole(role);

    // Show splash layer when a splash role is set
    if (role == ShellWindow::SplashRole)
        Q_EMIT m_compositor->fadeOut();
}

void PlasmaSurface::surface_set_flags(Resource *resource,
                                      uint32_t wlFlags)
{
    Q_UNUSED(resource);

    ShellWindow::Flags flags = wl2Flags(wlFlags);
    m_window->setFlags(flags);
}

void PlasmaSurface::surface_set_screen_edge(Resource *resource,
                                            uint32_t screen_edge)
{
    Q_UNUSED(resource);
    Q_UNUSED(screen_edge);
}

}
