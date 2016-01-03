/****************************************************************************
 * This file is part of Hawaii.
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

#include <waylandcompositor/wayland_wrapper/qwlsurface_p.h>

#include <GreenIsland/Server/ClientWindow>

#include "xwayland.h"
#include "xwaylandsurface.h"

namespace GreenIsland {

XWaylandSurface::XWaylandSurface(XWaylandWindow *window)
    : QObject(window)
    , QWaylandSurfaceInterface(window->surface())
{
    // This is a toplevel window by default
    surface()->handle()->setTransientParent(Q_NULLPTR);
    surface()->handle()->setTransientOffset(0, 0);
    setSurfaceType(QWaylandSurface::Toplevel);

    // Create client window
    m_window = new ClientWindow(surface(), this);

    // Surface events
    connect(surface(), &QWaylandSurface::configure, this, [this](bool hasBuffer) {
        surface()->setMapped(hasBuffer);
    });
}

XWaylandSurface::~XWaylandSurface()
{
}

void XWaylandSurface::setAppId(const QString &id)
{
    qCDebug(XWAYLAND_TRACE) << "Set appId to" << id;
    QWaylandSurfaceInterface::setSurfaceClassName(id);
}

void XWaylandSurface::setTitle(const QString &title)
{
    qCDebug(XWAYLAND_TRACE) << "Set title to" << title;
    QWaylandSurfaceInterface::setSurfaceTitle(title);
}

void XWaylandSurface::setType(const QWaylandSurface::WindowType &type)
{
    QWaylandSurfaceInterface::setSurfaceType(type);
}

bool XWaylandSurface::runOperation(QWaylandSurfaceOp *op)
{
    Q_UNUSED(op)
    return false;
}

}
