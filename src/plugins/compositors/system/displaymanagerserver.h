/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef DISPLAYMANAGERSERVER_H
#define DISPLAYMANAGERSERVER_H

#include <QtCompositor/wlcompositor.h>

#include "wayland-display-manager-server-protocol.h"
#include "systemcompositorserver.h"

class DisplayManagerServer
{
public:
    explicit DisplayManagerServer(SystemCompositorServer *compServer,
                                  Wayland::Compositor *handle);

private:
    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    static void destroy_resource(wl_resource *resource);

    static void terminate_client(struct wl_client *client,
                                 struct wl_resource *resource);

    static void add_client(struct wl_client *client,
                           struct wl_resource *resource,
                           uint32_t id, int32_t fd);
    static void switch_to_client(struct wl_client *client,
                                 struct wl_resource *resource,
                                 struct wl_resource *id);
    static void bind_key(struct wl_client *client,
                         struct wl_resource *resource,
                         int32_t key, uint32_t modifier,
                         uint32_t cookie);
    static void unbind_key(struct wl_client *client,
                           struct wl_resource *resource,
                           int32_t key, uint32_t modifier,
                           uint32_t cookie);

    static const struct wl_system_client_interface system_client_implementation;
    static const struct wl_display_manager_interface interface;

    SystemCompositorServer *compServer;
    Wayland::Compositor *handle;
    QList<wl_resource *> resources;
};

#endif // DISPLAYMANAGERSERVER_H
