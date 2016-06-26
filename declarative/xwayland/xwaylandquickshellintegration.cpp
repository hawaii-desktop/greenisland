/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL3GPL2$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPLv2 included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
