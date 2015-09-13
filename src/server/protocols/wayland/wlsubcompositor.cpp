/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "wlsubcompositor.h"
#include "wlsubsurface.h"

Q_LOGGING_CATEGORY(WLSUBCOMPOSITOR_TRACE, "greenisland.protocols.wlsubcompositor.trace")

namespace GreenIsland {

WlSubCompositorGlobal::WlSubCompositorGlobal(QObject *parent)
    : QObject(parent)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;
}

const wl_interface *WlSubCompositorGlobal::interface() const
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    return &wl_subcompositor_interface;
}

void WlSubCompositorGlobal::bind(wl_client *client, uint32_t version, uint32_t id)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    new WlSubCompositor(client, id, version, this);
}

WlSubCompositor::WlSubCompositor(wl_client *client, uint32_t name, uint32_t version, QObject *parent)
    : QObject(parent)
    , QtWaylandServer::wl_subcompositor(client, name, version)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;
}

WlSubCompositor::~WlSubCompositor()
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
}

void WlSubCompositor::subcompositor_destroy_resource(Resource *resource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    delete this;
}

void WlSubCompositor::subcompositor_destroy(Resource *resource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

void WlSubCompositor::subcompositor_get_subsurface(Resource *resource, uint32_t id,
                                                   wl_resource *surfaceResource,
                                                   wl_resource *parentResource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    static const char where[] = "get_subsurface: wl_subsurface@";

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    QWaylandSurface *parentSurface = QWaylandSurface::fromResource(parentResource);

    if (surface == parentSurface) {
        wl_resource_post_error(resource->handle, error_bad_surface,
                               "%s%d: wl_surface@%d cannot be its own parent",
                               where, id, wl_resource_get_id(surfaceResource));
        return;
    }

    if (!surface) {
        wl_resource_post_error(resource->handle, error_bad_surface,
                               "%s%d: wl_surface@%d not found",
                               where, id, wl_resource_get_id(surfaceResource));
        return;
    }

    if (!parentSurface) {
        wl_resource_post_error(resource->handle, error_bad_surface,
                               "%s%d: parent wl_surface@%d not found",
                               where, id, wl_resource_get_id(parentResource));
        return;
    }

    // TODO: Post error_bad_surface if the surface already has a role
    if (0) {
        wl_resource_post_error(resource->handle, error_bad_surface,
                               "%s%d: wl_surface@%d is already a sub-surface",
                               where, id, wl_resource_get_id(surfaceResource));
        return;
    }

    if (mainSurface(parentSurface) == surface) {
        wl_resource_post_error(resource->handle, error_bad_surface,
                               "%s%d: wl_surface@%d is an ancestor of parent",
                               where, id, wl_resource_get_id(surfaceResource));
        return;
    }

    WlSubSurface *subSurface =
            new WlSubSurface(this,
                             surface, parentSurface,
                             resource->client(), id,
                             resource->version());
    m_subSurfaces.append(subSurface);
}

WlSubSurface *WlSubCompositor::toSubSurface(QWaylandSurface *surface)
{
    Q_FOREACH (WlSubSurface *sub, m_subSurfaces) {
        if (sub->surface() == surface)
            return sub;
    }

    return Q_NULLPTR;
}

QWaylandSurface *WlSubCompositor::mainSurface(QWaylandSurface *surface)
{
    WlSubSurface *sub;

    while (surface && (sub = toSubSurface(surface)))
        surface = sub->parentSurface();

    return surface;
}

}
