/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>

#include "xwaylandquickshellintegration.h"
#include "xwaylandquickshellsurfaceitem.h"

XWaylandQuickShellIntegration::XWaylandQuickShellIntegration(XWaylandQuickShellSurfaceItem *item)
    : m_item(item)
    , m_shellSurface(item->shellSurface())
    , grabberState(GrabberState::Default)
{
    m_item->setSurface(m_shellSurface->surface());

    connect(m_shellSurface, &XWaylandShellSurface::startMove,
            this, &XWaylandQuickShellIntegration::handleStartMove);
    connect(m_shellSurface, &XWaylandShellSurface::startResize,
            this, &XWaylandQuickShellIntegration::handleStartResize);
}

bool XWaylandQuickShellIntegration::mouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        if (!resizeState.initialized) {
            resizeState.initialMousePos = event->windowPos();
            resizeState.initialized = true;
            return true;
        }
        float scaleFactor = m_item->view()->output()->scaleFactor();
        QPointF delta = (event->windowPos() - resizeState.initialMousePos) / scaleFactor;
        QSize newSize = m_shellSurface->sizeForResize(resizeState.initialSize, delta, resizeState.resizeEdges);
        m_shellSurface->sendConfigure(newSize);
    } else if (grabberState == GrabberState::Move) {
        QQuickItem *moveItem = m_item->moveItem();
        if (!moveState.initialized) {
            moveState.initialOffset = moveItem->mapFromItem(nullptr, event->windowPos());
            moveState.initialized = true;
            return true;
        }
        if (!moveItem->parentItem())
            return true;
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, event->windowPos());
        moveItem->setPosition(parentPos - moveState.initialOffset);
    }

    return false;
}

bool XWaylandQuickShellIntegration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

void XWaylandQuickShellIntegration::handleStartMove()
{
    grabberState = GrabberState::Move;
    moveState.initialized = false;
}

void XWaylandQuickShellIntegration::handleStartResize(XWaylandShellSurface::ResizeEdge edges)
{
    grabberState = GrabberState::Resize;
    resizeState.resizeEdges = edges;
    float scaleFactor = m_item->view()->output()->scaleFactor();
    resizeState.initialSize = m_shellSurface->surface()->size() / scaleFactor;
    resizeState.initialized = false;
}

#include "moc_xwaylandquickshellintegration.cpp"
