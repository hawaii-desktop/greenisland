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

#ifndef WLSUBSURFACE_H
#define WLSUBSURFACE_H

#include "surface.h"
#include "surfaceitem.h"
#include "surfaceinterface.h"

#include "wlsubcompositor.h"

namespace GreenIsland {

class Output;

class WlSubSurface : public QObject, public SurfaceInterface, public QtWaylandServer::wl_subsurface
{
public:
    enum Mode {
        Synchronized = 0,
        Desynchronized
    };

    WlSubSurface(WlSubCompositor *subCompositor,
                 Surface *surface, Surface *parentSurface,
                 wl_client *client, uint32_t id, uint32_t version);
    ~WlSubSurface();

    Surface *parentSurface() const;

    Mode mode() const;
    QPoint position() const;

    inline SurfaceItem *viewForOutput(Output *output) const {
        if (m_views.contains(output))
            return m_views.value(output);
        return Q_NULLPTR;
    }

protected:
    bool runOperation(SurfaceOperation *op) Q_DECL_OVERRIDE;

    void subsurface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void subsurface_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void subsurface_set_position(Resource *resource, int32_t x, int32_t y) Q_DECL_OVERRIDE;
    void subsurface_place_above(Resource *resource,
                                wl_resource *siblingResource) Q_DECL_OVERRIDE;
    void subsurface_place_below(Resource *resource,
                                wl_resource *siblingResource) Q_DECL_OVERRIDE;
    void subsurface_set_sync(Resource *resource) Q_DECL_OVERRIDE;
    void subsurface_set_desync(Resource *resource) Q_DECL_OVERRIDE;

private:
    WlSubCompositor *m_subCompositor;
    Surface *m_surface;
    Surface *m_parentSurface;
    Mode m_mode;
    QPoint m_pos;
    QHash<Output *, SurfaceItem *> m_views;

    WlSubSurface *fromSurface(Surface *surface);
    WlSubSurface *siblingCheck(Surface *surface,
                               const QByteArray &request);
    bool isSynchronized() const;
};

}

#endif // WLSUBSURFACE_H
