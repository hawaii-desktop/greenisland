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

#ifndef SYSTEMCOMPOSITORSERVER_H
#define SYSTEMCOMPOSITORSERVER_H

#include <QtCompositor/wlcompositor.h>
#include <wayland-util.h>

#include "wayland-system-compositor-server-protocol.h"
#include "systemcompositor.h"
#include "systemclient.h"

class SystemCompositorServer
{
public:
    explicit SystemCompositorServer(SystemCompositor *compositor,
                                    Wayland::Compositor *handle);

    void addSystemClient(SystemClient *client);

private:
    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    static void destroy_resource(wl_resource *resource);

    static void present_surface(struct wl_client *client,
                                struct wl_resource *resource,
                                struct wl_resource *surface,
                                struct wl_resource *output_resource);

    static void ready(struct wl_client *client,
                      struct wl_resource *resource);

    static const struct wl_system_compositor_interface interface;

    SystemCompositor *compositor;
    Wayland::Compositor *handle;
    SystemClientList m_systemClients;
    QList<wl_resource *> resources;
};

#endif // SYSTEMCOMPOSITORSERVER_H
