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

#include "abstractcompositor.h"
#include "surface_p.h"

#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "output.h"
#include "windowview.h"
#include "wlsubsurface.h"

namespace GreenIsland {

WlSubSurface::WlSubSurface(WlSubCompositor *subCompositor,
                           Surface *surface, Surface *parentSurface,
                           wl_client *client, uint32_t id, uint32_t version)
    : QObject(subCompositor)
    , SurfaceInterface(surface)
    , QtWaylandServer::wl_subsurface(client, id, version)
    , m_subCompositor(subCompositor)
    , m_surface(surface)
    , m_parentSurface(parentSurface)
    , m_mode(Synchronized)
    , m_pos(QPoint(0, 0))
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    // Compositor
    Compositor *compositor = static_cast<Compositor *>(m_surface->compositor());

    // Find the parent view
    ClientWindow *found = Q_NULLPTR;
    Q_FOREACH (ClientWindow *window, compositor->d_func()->clientWindowsList) {
        if (window->surface() == parentSurface) {
            found = window;
            break;
        }
    }
    // TODO: Perhaps add the parent view to the sub-surfaces list

    // Create a view for each output
    Q_FOREACH (AbstractOutput *wlOutput, m_surface->compositor()->outputs()) {
        Output *output = static_cast<Output *>(wlOutput);
        m_views[output] = static_cast<SurfaceItem *>(
                    m_surface->compositor()->createView(surface));
        if (found)
            m_views[output]->setParentItem(found->viewForOutput(output));
    }

    // Surface events
    connect(m_surface, &Surface::configure, this, [this](bool hasBuffer) {
        // Map or unmap the surface
        m_surface->setMapped(hasBuffer);
    }, Qt::QueuedConnection);
    connect(m_parentSurface, &Surface::surfaceDestroyed, this, [this] {
        // Destroy all views
        QList<Output *> outputs = m_views.keys();
        Q_FOREACH (Output *output, outputs)
            m_views.take(output)->deleteLater();

        // Unmap
        if (m_surface->isMapped())
            m_surface->setMapped(false);
    }, Qt::QueuedConnection);
}

WlSubSurface::~WlSubSurface()
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    m_subCompositor->m_subSurfaces.removeOne(this);

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);

    QList<Output *> outputs = m_views.keys();
    Q_FOREACH (Output *output, outputs)
        m_views.take(output)->deleteLater();
}

Surface *WlSubSurface::parentSurface() const
{
    return m_parentSurface;
}

WlSubSurface::Mode WlSubSurface::mode() const
{
    return m_mode;
}

QPoint WlSubSurface::position() const
{
    return m_pos;
}

bool WlSubSurface::runOperation(SurfaceOperation *op)
{
    Q_UNUSED(op)
    return false;
}

void WlSubSurface::subsurface_destroy_resource(Resource *resource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    delete this;
}

void WlSubSurface::subsurface_destroy(Resource *resource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

void WlSubSurface::subsurface_set_position(Resource *resource, int32_t x, int32_t y)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    m_pos = QPoint(x, y);

    QList<SurfaceItem *> views = m_views.values();
    Q_FOREACH (SurfaceItem *view, views)
        view->setPosition(QPointF(m_pos));
}

void WlSubSurface::subsurface_place_above(Resource *resource,
                                          wl_resource *siblingResource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    Surface *sibling = Surface::fromResource(siblingResource);

    WlSubSurface *subSibling = siblingCheck(sibling, QByteArray("place_above"));
    if (!subSibling)
        return;

    QList<SurfaceItem *> views = m_views.values();
    Q_FOREACH (SurfaceItem *view, views)
        view->setZ(1);
}

void WlSubSurface::subsurface_place_below(Resource *resource,
                                          wl_resource *siblingResource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    Surface *sibling = Surface::fromResource(siblingResource);

    WlSubSurface *subSibling = siblingCheck(sibling, QByteArray("place_below"));
    if (!subSibling)
        return;

    QList<SurfaceItem *> views = m_views.values();
    Q_FOREACH (SurfaceItem *view, views)
        view->setZ(0);
}

void WlSubSurface::subsurface_set_sync(Resource *resource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    m_mode = Synchronized;
}

void WlSubSurface::subsurface_set_desync(Resource *resource)
{
    qCDebug(WLSUBCOMPOSITOR_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    if (m_mode == Synchronized) {
        m_mode = Desynchronized;
        // TODO: We really need a QWaylandSubSurface
        //if (!isSynchronized())
            //synchronizedCommit();
    }
}

WlSubSurface *WlSubSurface::fromSurface(Surface *surface)
{
    WlSubSurface *sub = m_subCompositor->toSubSurface(surface);
    if (sub)
        return sub;

    Q_FOREACH (WlSubSurface *sub, m_subCompositor->subSurfaces()) {
        if (sub->surface() == surface)
            return sub;
    }

    return Q_NULLPTR;
}

WlSubSurface *WlSubSurface::siblingCheck(Surface *surface,
                           const QByteArray &request)
{
    WlSubSurface *sibling = fromSurface(surface);
    if (!sibling) {
        wl_resource_post_error(resource()->handle, error_bad_surface,
                               "%s: wl_surface@%d is not a parent or sibling",
                               request.constData(),
                               wl_resource_get_id(surface->handle()->resource()->handle));
        return Q_NULLPTR;
    }

    if (sibling->parentSurface() != parentSurface()) {
        wl_resource_post_error(resource()->handle, error_bad_surface,
                               "%s: wl_surface@%d has a different parent",
                               request.constData(),
                               wl_resource_get_id(surface->handle()->resource()->handle));
        return Q_NULLPTR;
    }

    return sibling;
}

bool WlSubSurface::isSynchronized() const
{
    WlSubSurface *sub = const_cast<WlSubSurface *>(this);

    while (sub) {
        if (sub->mode() == Synchronized)
            return true;
        if (!sub->parentSurface())
            return false;
        sub = m_subCompositor->toSubSurface(sub->parentSurface());
    }

    return false;
}

}
