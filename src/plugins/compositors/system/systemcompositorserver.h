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

#include <QtCompositor/private/qwlcompositor_p.h>
#include <wayland-util.h>

#include "wayland-system-compositor-server-protocol.h"
#include "wayland-display-manager-server-protocol.h"
#include "systemcompositor.h"
#include "systemclient.h"

class SystemCompositorServer
{
public:
    explicit SystemCompositorServer(SystemCompositor *compositor,
                                    QtWayland::Compositor *handle);

    SystemCompositor *compositor;
    QtWayland::Compositor *handle;
    wl_client *dmClient;
    wl_resource *dmResource;
    SystemClientList systemClients;
    QList<wl_resource *> resources;

private:
    // System compositor interface
    static void bindSystemCompositor(wl_client *client, void *data,
                                     uint32_t version, uint32_t id);
    static void unbindSystemCompositor(wl_resource *resource);

    // Display manager interface
    static void bindDisplayManager(wl_client *client, void *data,
                                   uint32_t version, uint32_t id);
    static void unbindDisplayManager(wl_resource *resource);

    // System compositor server-side implementation
    static void handlePresentSurface(wl_client *client,
                                     wl_resource *resource,
                                     wl_resource *surface,
                                     wl_resource *output_resource);
    static void handleReady(wl_client *client,
                            wl_resource *resource);

    // Display manager server-side implementation
    static void unbindSystemClient(wl_resource *resource);

    static void handleAddClient(wl_client *client,
                                wl_resource *resource,
                                uint32_t id, int32_t fd);
    static void handleSwitchToClient(wl_client *client,
                                     wl_resource *resource,
                                     wl_resource *id);
    static void handleBindKey(wl_client *client,
                              wl_resource *resource,
                              uint32_t key, uint32_t modifier, uint32_t cookie);
    static void handleUnbindKey(wl_client *client,
                                wl_resource *resource,
                                uint32_t key, uint32_t modifier, uint32_t cookie);

    // System client server-side implementation
    static void handleTerminateClient(wl_client *client,
                                      wl_resource *resource);

    // Interfaces
    static const struct wl_system_compositor_interface systemCompositorInterface;
    static const struct wl_display_manager_interface displayManagerInterface;
    static const struct wl_system_client_interface systemClientInterface;
};

#endif // SYSTEMCOMPOSITORSERVER_H
