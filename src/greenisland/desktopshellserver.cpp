/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include <QtCompositor/wlsurface.h>
#include <QtCompositor/waylandsurface.h>

#include "desktopshellserver.h"

const struct desktop_shell_interface DesktopShellServer::shell_interface = {
    DesktopShellServer::set_background,
    DesktopShellServer::set_panel,
    DesktopShellServer::set_panel_pos,
    DesktopShellServer::set_launcher,
    DesktopShellServer::set_launcher_pos
};

DesktopShellServer::DesktopShellServer(Wayland::Compositor *compositor)
    : m_compositor(compositor)
    , m_panelSurface(0)
    , m_launcherSurface(0)
{
    wl_display_add_global(compositor->wl_display(),
                          &desktop_shell_interface, this,
                          DesktopShellServer::bind_func);
}

DesktopShellServer::~DesktopShellServer()
{
}

void DesktopShellServer::bind_func(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);

    wl_resource *resource = wl_client_add_object(client, &desktop_shell_interface, &shell_interface, id, data);
    resource->destroy = destroy_resource;

    DesktopShellServer *self = static_cast<DesktopShellServer *>(resource->data);
    self->m_resources.append(resource);
}

void DesktopShellServer::destroy_resource(wl_resource *resource)
{
    DesktopShellServer *self = static_cast<DesktopShellServer *>(resource->data);
    self->m_resources.removeOne(resource);
    free(resource);
}

void DesktopShellServer::set_background(struct wl_client *client,
                                        struct wl_resource *resource,
                                        const char *uri)
{
}

void DesktopShellServer::set_panel(struct wl_client *client,
                                   struct wl_resource *resource,
                                   struct wl_resource *surface)
{
    DesktopShellServer *self = static_cast<DesktopShellServer *>(resource->data);
    self->m_panelSurface = Wayland::resolve<Wayland::Surface>(surface);
    if (!self->m_panelSurface)
        return;
    self->m_panelSurface->waylandSurface()->setWindowProperty("special", true);
}

void DesktopShellServer::set_panel_pos(struct wl_client *client,
                                       struct wl_resource *resource,
                                       uint32_t x, uint32_t y)
{
    DesktopShellServer *self = static_cast<DesktopShellServer *>(resource->data);
    self->m_panelSurface->setPos(QPointF(x, y));
}

void DesktopShellServer::set_launcher(struct wl_client *client,
                                      struct wl_resource *resource,
                                      struct wl_resource *surface)
{
    DesktopShellServer *self = static_cast<DesktopShellServer *>(resource->data);
    self->m_launcherSurface = Wayland::resolve<Wayland::Surface>(surface);
    if (!self->m_launcherSurface)
        return;
    self->m_launcherSurface->waylandSurface()->setWindowProperty("special", true);
}

void DesktopShellServer::set_launcher_pos(struct wl_client *client,
                                          struct wl_resource *resource,
                                          uint32_t x, uint32_t y)
{
    DesktopShellServer *self = static_cast<DesktopShellServer *>(resource->data);
    self->m_launcherSurface->setPos(QPointF(x, y));
}
