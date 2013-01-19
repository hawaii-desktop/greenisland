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

#include <QtCompositor/wlsurface.h>

#include "systemcompositorserver.h"
#include "wayland-display-manager-server-protocol.h"

const struct wl_system_compositor_interface SystemCompositorServer::interface = {
    SystemCompositorServer::present_surface,
    SystemCompositorServer::ready
};

SystemCompositorServer::SystemCompositorServer(SystemCompositor *compositor,
                                               Wayland::Compositor *handle)
    : compositor(compositor)
    , handle(handle)
{
    wl_display_add_global(handle->wl_display(),
                          &wl_system_compositor_interface, this,
                          SystemCompositorServer::bind_func);
}

void SystemCompositorServer::addSystemClient(SystemClient *client)
{
    m_systemClients.append(client);
}

void SystemCompositorServer::bind_func(struct wl_client *client, void *data,
                                       uint32_t version, uint32_t id)
{
    Q_ASSERT(version == 1);

    wl_resource *resource = wl_client_add_object(client, &wl_system_compositor_interface,
                                                 &interface, id, data);
    resource->destroy = destroy_resource;

    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    self->resources.append(resource);
}

void SystemCompositorServer::destroy_resource(struct wl_resource *resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    self->resources.removeOne(resource);
    free(resource);
}

void SystemCompositorServer::present_surface(struct wl_client *client,
                                             struct wl_resource *resource,
                                             struct wl_resource *surface,
                                             struct wl_resource *output_resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    Wayland::Output *output = static_cast<Wayland::Output *>(output_resource->data);

    // Find a system client associated with the Wayland client passed to this function
    SystemClient *systemClient = self->compositor->systemClientForClient(client);
    if (!systemClient) {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "non-system client tried to signal readiness");
        return;
    }

    // Find an already mapped surface
    WaylandSurface *mappedSurface = systemClient->surfaceForOutput(output);
    if (!mappedSurface) {
        mappedSurface = new WaylandSurface(Wayland::resolve<Wayland::Surface>(surface));
        systemClient->mapSurfaceToOutput(mappedSurface, output);
    }

    // System client is full screen
    QRect geometry = self->compositor->outputGeometry();
    mappedSurface->setPos(geometry.topLeft());
    mappedSurface->setSize(geometry.size());
}

void SystemCompositorServer::ready(struct wl_client *client,
                                   struct wl_resource *resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);

    SystemClient *systemClient = self->compositor->systemClientForClient(client);
    if (!systemClient) {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "non-system client tried to signal readiness");
        return;
    }

    // System clients can signal readiness only once
    if (systemClient->isReadySent()) {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "system clean was already ready");
        return;
    }

    // Set readiness flag
    systemClient->setReady();
    wl_system_client_send_ready(systemClient->clientResource());
}
