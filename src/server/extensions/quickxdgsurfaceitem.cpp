/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>

#include "quickxdgsurfaceitem.h"
#include "quickxdgsurfaceitem_p.h"

namespace GreenIsland {

namespace Server {

QuickXdgSurfaceItem::QuickXdgSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QuickXdgSurfaceItemPrivate(), parent)
{
}

QuickXdgSurfaceItem::QuickXdgSurfaceItem(QuickXdgSurfaceItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
}

XdgSurface *QuickXdgSurfaceItem::shellSurface() const
{
    Q_D(const QuickXdgSurfaceItem);
    return d->shellSurface;
}

void QuickXdgSurfaceItem::setShellSurface(XdgSurface *shellSurface)
{
    Q_D(QuickXdgSurfaceItem);

    if (shellSurface == d->shellSurface)
        return;

    if (d->shellSurface) {
        disconnect(d->shellSurface, &XdgSurface::startMove,
                   this, &QuickXdgSurfaceItem::handleStartMove);
        disconnect(d->shellSurface, &XdgSurface::startResize,
                   this, &QuickXdgSurfaceItem::handleStartResize);
    }

    d->shellSurface = shellSurface;
    if (d->shellSurface) {
        connect(d->shellSurface, &XdgSurface::startMove,
                this, &QuickXdgSurfaceItem::handleStartMove);
        connect(d->shellSurface, &XdgSurface::startResize,
                this, &QuickXdgSurfaceItem::handleStartResize);
    }

    Q_EMIT shellSurfaceChanged();
}

QQuickItem *QuickXdgSurfaceItem::moveItem() const
{
    Q_D(const QuickXdgSurfaceItem);
    return d->moveItem;
}

void QuickXdgSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(QuickXdgSurfaceItem);

    if (d->moveItem == moveItem)
        return;

    d->moveItem = moveItem;
    Q_EMIT moveItemChanged();
}

void QuickXdgSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QuickXdgSurfaceItem);

    if (d->grabberState == QuickXdgSurfaceItemPrivate::ResizeState) {
        Q_ASSERT(d->resizeState.inputDevice == compositor()->inputDeviceFor(event));

        if (!d->resizeState.initialized) {
            d->resizeState.initialMousePos = event->windowPos();
            d->resizeState.initialized = true;
            return;
        }

        QPointF delta = event->windowPos() - d->resizeState.initialMousePos;
        QSize newSize = shellSurface()->sizeForResize(d->resizeState.initialSize, delta, d->resizeState.resizeEdges);
        shellSurface()->sendConfigure(newSize + d->resizeState.deltaSize);
    } else if (d->grabberState == QuickXdgSurfaceItemPrivate::MoveState) {
        Q_ASSERT(d->moveState.inputDevice == compositor()->inputDeviceFor(event));

        QQuickItem *moveItem = d->moveItem ? d->moveItem : this;

        if (!d->moveState.initialized) {
            d->moveState.initialOffset = moveItem->mapFromItem(Q_NULLPTR, event->windowPos());
            d->moveState.initialized = true;
            return;
        }

        if (!moveItem->parentItem())
            return;

        QPointF parentPos = moveItem->parentItem()->mapFromItem(Q_NULLPTR, event->windowPos());
        QPointF pos = parentPos - d->moveState.initialOffset;

        // Moving a maximized window will restore its size
        if (shellSurface()->isMaximized()) {
            QPointF threshold = d->moveState.initialOffset + QPointF(20, 20);
            /*
            if (pos.x() >= threshold.x() || pos.y() >= threshold.y())
                shellSurface()->restoreSize();
            */
        }

        moveItem->setPosition(pos);
    } else {
        QWaylandQuickItem::mouseMoveEvent(event);
    }
}

void QuickXdgSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QuickXdgSurfaceItem);

    if (d->grabberState != QuickXdgSurfaceItemPrivate::DefaultState) {
        d->grabberState = QuickXdgSurfaceItemPrivate::DefaultState;
        return;
    }

    shellSurface()->ping();

    QWaylandQuickItem::mouseReleaseEvent(event);
}

void QuickXdgSurfaceItem::surfaceChangedEvent(QWaylandSurface *newSurface,
                                              QWaylandSurface *oldSurface)
{
    if (oldSurface)
        disconnect(oldSurface, &QWaylandSurface::offsetForNextFrame,
                   this, &QuickXdgSurfaceItem::adjustOffsetForNextFrame);
    if (newSurface)
        connect(newSurface, &QWaylandSurface::offsetForNextFrame,
                this, &QuickXdgSurfaceItem::adjustOffsetForNextFrame);
}

void QuickXdgSurfaceItem::componentComplete()
{
    Q_D(QuickXdgSurfaceItem);

    if (!d->shellSurface)
        setShellSurface(new XdgSurface());

    QWaylandQuickItem::componentComplete();
}

void QuickXdgSurfaceItem::handleStartMove(QWaylandInputDevice *inputDevice)
{
    Q_D(QuickXdgSurfaceItem);
    d->grabberState = QuickXdgSurfaceItemPrivate::MoveState;
    d->moveState.inputDevice = inputDevice;
    d->moveState.initialized = false;
}

void QuickXdgSurfaceItem::handleStartResize(QWaylandInputDevice *inputDevice, XdgSurface::ResizeEdge edge)
{
    Q_D(QuickXdgSurfaceItem);
    d->grabberState = QuickXdgSurfaceItemPrivate::ResizeState;
    d->resizeState.inputDevice = inputDevice;
    d->resizeState.resizeEdges = edge;
    if (shellSurface()->windowGeometry().isValid()) {
        d->resizeState.initialSize = shellSurface()->windowGeometry().size();
        d->resizeState.deltaSize = surface()->size() - shellSurface()->windowGeometry().size();
    } else {
        d->resizeState.initialSize = surface()->size();
    }
    d->resizeState.initialized = false;
}

void QuickXdgSurfaceItem::adjustOffsetForNextFrame(const QPointF &offset)
{
    Q_D(QuickXdgSurfaceItem);
    QQuickItem *moveItem = d->moveItem ? d->moveItem : this;
    moveItem->setPosition(moveItem->position() + offset);
}

} // namespace Server

} // namespace GreenIsland

#include "moc_quickxdgsurfaceitem.cpp"
