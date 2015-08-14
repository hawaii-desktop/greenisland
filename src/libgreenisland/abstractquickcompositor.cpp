/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Giulio Camuffo
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwlsurface_p.h"

#include "clientconnection.h"
#include "abstractquickcompositor.h"
#include "quicksurface.h"
#include "surfaceitem.h"
#include "quickoutput.h"

namespace GreenIsland {

class AbstractQuickCompositorPrivate : public GreenIsland::WlCompositor
{
public:
    AbstractQuickCompositorPrivate(AbstractQuickCompositor *compositor, AbstractCompositor::ExtensionFlags extensions)
        : GreenIsland::WlCompositor(compositor, extensions)
    {
    }

    void compositor_create_surface(wl_compositor::Resource *resource, uint32_t id) Q_DECL_OVERRIDE
    {
        QuickSurface *surface = new QuickSurface(resource->client(), id, wl_resource_get_version(resource->handle), static_cast<AbstractQuickCompositor *>(m_qt_compositor));
        surface->handle()->addToOutput(primaryOutput()->handle());
        m_surfaces << surface->handle();
        //BUG: This may not be an on-screen window surface though
        m_qt_compositor->surfaceCreated(surface);
    }
};


AbstractQuickCompositor::AbstractQuickCompositor(const char *socketName, ExtensionFlags extensions)
                       : AbstractCompositor(socketName, new AbstractQuickCompositorPrivate(this, extensions))
{
    qmlRegisterUncreatableType<SurfaceItem>("QtCompositor", 1, 0, "WaylandSurfaceItem", QObject::tr("Cannot create instance of WaylandSurfaceItem"));
    qmlRegisterUncreatableType<QuickSurface>("QtCompositor", 1, 0, "WaylandQuickSurface", QObject::tr("Cannot create instance of WaylandQuickSurface"));
    qmlRegisterUncreatableType<ClientConnection>("QtCompositor", 1, 0, "WaylandClient", QObject::tr("Cannot create instance of WaylandClient"));
    qmlRegisterUncreatableType<AbstractOutput>("QtCompositor", 1, 0, "WaylandOutput", QObject::tr("Cannot create instance of WaylandOutput"));
}

SurfaceView *AbstractQuickCompositor::createView(Surface *surf)
{
    return new SurfaceItem(static_cast<QuickSurface *>(surf));
}

AbstractOutput *AbstractQuickCompositor::createOutput(QWindow *window,
                                                      const QString &manufacturer,
                                                      const QString &model)
{
    QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(window);
    if (!quickWindow)
        qFatal("%s: couldn't cast QWindow to QQuickWindow. All output windows must "
               "be QQuickWindow derivates when using AbstractQuickCompositor", Q_FUNC_INFO);
    return new QuickOutput(this, quickWindow, manufacturer, model);
}

}
