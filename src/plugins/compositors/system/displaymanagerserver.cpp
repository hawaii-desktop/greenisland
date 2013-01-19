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
#include <QtCompositor/waylandsurface.h>
#include <QtCompositor/waylandsurfaceitem.h>

#include "displaymanagerserver.h"
#include "systemclient.h"

const struct wl_system_client_interface DisplayManagerServer::system_client_implementation = {
    DisplayManagerServer::terminate_client
};

const struct wl_display_manager_interface DisplayManagerServer::interface = {
    DisplayManagerServer::add_client,
    DisplayManagerServer::switch_to_client,
    DisplayManagerServer::bind_key,
    DisplayManagerServer::unbind_key
};

DisplayManagerServer::DisplayManagerServer(SystemCompositorServer *compServer,
                                           Wayland::Compositor *handle)
    : compServer(compServer)
    , handle(handle)
{
    wl_display_add_global(handle->wl_display(),
                          &wl_display_manager_interface, this,
                          DisplayManagerServer::bind_func);
}

void DisplayManagerServer::bind_func(struct wl_client *client, void *data,
                                     uint32_t version, uint32_t id)
{
    Q_ASSERT(version == 1);
}

void DisplayManagerServer::destroy_resource(struct wl_resource *resource)
{
    DisplayManagerServer *self = static_cast<DisplayManagerServer *>(
                resource->data);
    self->resources.removeOne(resource);
    free(resource);
}


void DisplayManagerServer::terminate_client(struct wl_client *client,
                                            struct wl_resource *resource)
{
    // TODO:
}

void DisplayManagerServer::add_client(struct wl_client *client,
                                      struct wl_resource *resource,
                                      uint32_t id, int32_t fd)
{
    DisplayManagerServer *self = static_cast<DisplayManagerServer *>(
                resource->data);

    struct wl_client *wlClient = wl_client_create(
                wl_client_get_display(client), fd);

    SystemClient *systemClient = new SystemClient(wlClient);

    struct wl_resource *clientRes =
            wl_client_add_object(client,
                                 &wl_system_client_interface,
                                 &system_client_implementation,
                                 id, systemClient);
    systemClient->setResource(clientRes);

    self->compServer->addSystemClient(systemClient);
}

void DisplayManagerServer::switch_to_client(struct wl_client *client,
                                            struct wl_resource *resource,
                                            struct wl_resource *id)
{
    Q_UNUSED(client);

    DisplayManagerServer *self = static_cast<DisplayManagerServer *>(
                resource->data);

    SystemClient *systemClient = static_cast<SystemClient *>(id->data);

    // TODO: Transition between clients

    WaylandSurfaceList surfaces = systemClient->surfaces();
    for (int i = 0; i < surfaces.size(); i++) {
        WaylandSurface *surface = surfaces.at(i);
        Wayland::Surface *handle = surface->handle();

        handle->damage(QRect(QPoint(0, 0), surface->size()));
        if (surface->surfaceItem())
            surface->surfaceItem()->takeFocus();
    }
}

void DisplayManagerServer::bind_key(struct wl_client *client,
                                    struct wl_resource *resource,
                                    int32_t key, uint32_t modifier,
                                    uint32_t cookie)
{
}

void DisplayManagerServer::unbind_key(struct wl_client *client,
                                      struct wl_resource *resource,
                                      int32_t key, uint32_t modifier,
                                      uint32_t cookie)
{
}
