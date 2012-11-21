/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef DESKTOPSHELLSERVER_H
#define DESKTOPSHELLSERVER_H

#include <QtCompositor/wlcompositor.h>

#include "wayland-desktop-extension-server-protocol.h"
#include "wayland-util.h"

class DesktopShellServer
{
public:
    DesktopShellServer(Wayland::Compositor *compositor);
    ~DesktopShellServer();

private:
    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    static void destroy_resource(wl_resource *resource);

    static void set_surface(struct wl_client *client,
                            struct wl_resource *resource,
                            struct wl_resource *surface);

    static void set_geometry(struct wl_client *client,
                             struct wl_resource *resource,
                             uint32_t x, uint32_t y,
                             uint32_t w, uint32_t h);

    static void set_background(struct wl_client *client,
                               struct wl_resource *resource,
                               const char *uri);

    static const struct desktop_shell_interface shell_interface;

    Wayland::Compositor *m_compositor;
    QList<wl_resource *> m_resources;
    Wayland::Surface *m_surface;
};

#endif // DESKTOPSHELLSERVER_H
