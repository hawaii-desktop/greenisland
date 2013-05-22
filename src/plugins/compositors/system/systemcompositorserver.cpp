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

#include <QDebug>
#include <QQuickItem>

#include <QtCompositor/private/qwlsurface_p.h>
#include <QtCompositor/private/qwloutput_p.h>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceItem>

#include "systemcompositorserver.h"
#include "wayland-system-compositor-client-protocol.h"
#include "wayland-display-manager-client-protocol.h"

const struct wl_system_compositor_interface SystemCompositorServer::systemCompositorInterface = {
    SystemCompositorServer::handlePresentSurface,
    SystemCompositorServer::handleReady
};

const struct wl_display_manager_interface SystemCompositorServer::displayManagerInterface = {
    SystemCompositorServer::handleAddClient,
    SystemCompositorServer::handleSwitchToClient,
    SystemCompositorServer::handleBindKey,
    SystemCompositorServer::handleUnbindKey
};

const struct wl_system_client_interface SystemCompositorServer::systemClientInterface = {
    SystemCompositorServer::handleTerminateClient
};

SystemCompositorServer::SystemCompositorServer(SystemCompositor *compositor,
                                               QtWayland::Compositor *handle)
    : compositor(compositor)
    , handle(handle)
    , dmClient(0)
    , dmResource(0)
{
    // Bind interfaces
    wl_display_add_global(handle->wl_display(),
                          &wl_system_compositor_interface, this,
                          SystemCompositorServer::bindSystemCompositor);
    wl_display_add_global(handle->wl_display(),
                          &wl_display_manager_interface, this,
                          SystemCompositorServer::bindDisplayManager);
}

void SystemCompositorServer::bindSystemCompositor(wl_client *client, void *data,
                                                  uint32_t version, uint32_t id)
{
    Q_ASSERT(version == 1);

    wl_resource *resource = wl_client_add_object(client, &wl_system_compositor_interface,
                                                 &systemCompositorInterface, id, data);
    resource->destroy = unbindSystemCompositor;

    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(data);
    self->resources.append(resource);
}

void SystemCompositorServer::unbindSystemCompositor(wl_resource *resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    self->resources.removeOne(resource);
    free(resource);
}

void SystemCompositorServer::bindDisplayManager(wl_client *client, void *data,
                                                uint32_t version, uint32_t id)
{
    Q_ASSERT(version == 1);

    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(data);

    wl_resource *resource = wl_client_add_object(client,
                                                 &wl_display_manager_interface,
                                                 &displayManagerInterface,
                                                 id, data);
    self->dmClient = client;
    self->dmResource = resource;
    resource->destroy = unbindDisplayManager;

    self->resources.append(resource);
}

void SystemCompositorServer::unbindDisplayManager(wl_resource *resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    self->resources.removeOne(resource);
    free(resource);

    self->dmClient = 0;
    self->dmResource = 0;
}

void SystemCompositorServer::handlePresentSurface(wl_client *client,
                                                  wl_resource *resource,
                                                  wl_resource *surface,
                                                  wl_resource *output_resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    QtWayland::Output *output = static_cast<QtWayland::Output *>(output_resource->data);

    // Find a system client associated with the Wayland client passed to this function
    SystemClient *systemClient = 0;
    for (int i = 0; i < self->systemClients.size(); i++) {
        if (self->systemClients.at(i)->client() == client) {
            systemClient = self->systemClients.at(i);
            break;
        }
    }
    if (!systemClient) {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "non-system client tried to signal readiness");
        return;
    }

    // Find an already mapped surface
    QWaylandSurface *mappedSurface = systemClient->surfaceForOutput(output);
    if (!mappedSurface) {
        mappedSurface = new QWaylandSurface(QtWayland::Surface::fromResource(surface));
        systemClient->mapSurfaceToOutput(mappedSurface, output);
    }

    // System client is full screen
    QRect geometry = self->compositor->outputGeometry();
    mappedSurface->setPos(geometry.topLeft());
    mappedSurface->setSize(geometry.size());
}

void SystemCompositorServer::handleReady(wl_client *client,
                                         wl_resource *resource)
{
    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);

    SystemClient *systemClient = 0;
    for (int i = 0; i < self->systemClients.size(); i++) {
        if (self->systemClients.at(i)->client() == client) {
            systemClient = self->systemClients.at(i);
            break;
        }
    }
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

void SystemCompositorServer::unbindSystemClient(wl_resource *resource)
{
    SystemClient *self = static_cast<SystemClient *>(resource->data);
    self->unbindClientResource();
    delete self;
}

void SystemCompositorServer::handleAddClient(wl_client *client,
                                             wl_resource *resource,
                                             uint32_t id, int32_t fd)
{
    wl_client *newClient = wl_client_create(wl_client_get_display(client), fd);
    SystemClient *systemClient = new SystemClient(newClient);

    struct wl_resource *newResource =
            wl_client_add_object(client,
                                 &wl_system_client_interface,
                                 &SystemCompositorServer::systemClientInterface,
                                 id, systemClient);
    newResource->destroy = unbindSystemClient;
    systemClient->setClientResource(newResource);
}

void SystemCompositorServer::handleSwitchToClient(wl_client *client,
                                                  wl_resource *resource,
                                                  wl_resource *id)
{
    Q_UNUSED(client);

    SystemCompositorServer *self = static_cast<SystemCompositorServer *>(resource->data);
    SystemClient *systemClient = static_cast<SystemClient *>(id->data);

    // TODO: Transition

    // Activate surface
    for (int i = 0; i < systemClient->surfaces().size(); i++) {
        QWaylandSurface *surface = systemClient->surfaces().at(i);

#if 0
        if (surface->hasShellSurface()) {
            QWaylandSurfaceItem *item = surface->surfaceItem();
            if (item)
                item->takeFocus();
        }
#else
        QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(self->compositor);
        compositor->handle()->setDirectRenderSurface(surface->handle(),
                                                     self->compositor->openglContext());
#endif
    }
}

void SystemCompositorServer::handleBindKey(wl_client *client,
                                           wl_resource *resource,
                                           uint32_t key, uint32_t modifier,
                                           uint32_t cookie)
{
    Q_UNUSED(resource);
    Q_UNUSED(client);
    Q_UNUSED(key);
    Q_UNUSED(modifier);
    Q_UNUSED(cookie);

    // TODO:
}

void SystemCompositorServer::handleUnbindKey(wl_client *client,
                                             wl_resource *resource,
                                             uint32_t key, uint32_t modifier,
                                             uint32_t cookie)
{
    Q_UNUSED(resource);
    Q_UNUSED(client);
    Q_UNUSED(key);
    Q_UNUSED(modifier);
    Q_UNUSED(cookie);

    // TODO:
}

void SystemCompositorServer::handleTerminateClient(wl_client *client,
                                                   wl_resource *resource)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
}
