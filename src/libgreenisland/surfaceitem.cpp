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

#include <QDebug>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandInputDevice>

#include "surfaceitem.h"

class SurfaceItemPrivate
{
public:
    SurfaceItemPrivate(SurfaceItem *parent)
        : q_ptr(parent)
    {
    }

protected:
    Q_DECLARE_PUBLIC(SurfaceItem)
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
QWaylandSurfaceItem::mousePressEvent(event);
return;
    if (surface()) {
        QWaylandInputDevice *inputDevice = surface()->compositor()->defaultInputDevice();
        if (inputDevice->mouseFocus() != surface())
            inputDevice->setMouseFocus(surface(), event->pos(), event->globalPos());
        inputDevice->sendMousePressEvent(event->button(), event->pos(), event->globalPos());
    } else {
        event->ignore();
    }
}

void SurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (surface() && surface()->surfaceItem()) {
qDebug() << "*******MOVE*" << event->pos() << event->globalPos();
surface()->surfaceItem()->takeFocus();
QWaylandSurfaceItem::mouseMoveEvent(event);
    } else {
        event->ignore();
    }
}

void SurfaceItem::touchEvent(QTouchEvent *event)
{
    // Continue with normal event handling
    QWaylandSurfaceItem::touchEvent(event);
}

#include "moc_surfaceitem.cpp"
