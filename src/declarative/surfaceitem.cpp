/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "surfaceitem.h"

class SurfaceItemPrivate
{
    Q_DECLARE_PUBLIC(SurfaceItem)
public:
    SurfaceItemPrivate(SurfaceItem *parent)
        : q_ptr(parent)
    {
    }

    SurfaceItem *const q_ptr;
};

SurfaceItem::SurfaceItem(QQuickItem *parent)
    : QWaylandSurfaceItem(parent)
    , d_ptr(new SurfaceItemPrivate(this))
{
}

SurfaceItem::SurfaceItem(QWaylandSurface *surface, QQuickItem *parent)
    : QWaylandSurfaceItem(surface, parent)
    , d_ptr(new SurfaceItemPrivate(this))
{
}

SurfaceItem::~SurfaceItem()
{
    delete d_ptr;
}

void SurfaceItem::mousePressEvent(QMouseEvent *event)
{
    // Continue with normal event handling
    QWaylandSurfaceItem::mousePressEvent(event);
}

void SurfaceItem::touchEvent(QTouchEvent *event)
{
    // Continue with normal event handling
    QWaylandSurfaceItem::touchEvent(event);
}

#include "moc_surfaceitem.cpp"
