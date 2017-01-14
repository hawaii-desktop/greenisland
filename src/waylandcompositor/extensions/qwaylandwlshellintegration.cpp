/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandwlshellintegration_p.h"

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandWlShellSurface>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <GreenIsland/QtWaylandCompositor/QWaylandSeat>

QT_BEGIN_NAMESPACE

namespace QtWayland {

WlShellIntegration::WlShellIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<QWaylandWlShellSurface *>(item->shellSurface()))
    , grabberState(GrabberState::Default)
    , isMaximized(false)
    , isFullScreen(false)
    , isPopup(false)
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface, &QWaylandWlShellSurface::startMove, this, &WlShellIntegration::handleStartMove);
    connect(m_shellSurface, &QWaylandWlShellSurface::startResize, this, &WlShellIntegration::handleStartResize);
    connect(m_shellSurface->surface(), &QWaylandSurface::redraw, this, &WlShellIntegration::redraw);
    connect(m_shellSurface->surface(), &QWaylandSurface::offsetForNextFrame, this, &WlShellIntegration::adjustOffsetForNextFrame);
    connect(m_shellSurface, &QWaylandWlShellSurface::setDefaultToplevel, this, &WlShellIntegration::handleSetDefaultTopLevel);
    connect(m_shellSurface, &QWaylandWlShellSurface::setTransient, this, &WlShellIntegration::handleSetTransient);
    connect(m_shellSurface, &QWaylandWlShellSurface::setMaximized, this, &WlShellIntegration::handleSetMaximized);
    connect(m_shellSurface, &QWaylandWlShellSurface::setFullScreen, this, &WlShellIntegration::handleSetFullScreen);
    connect(m_shellSurface, &QWaylandWlShellSurface::setPopup, this, &WlShellIntegration::handleSetPopup);
    connect(m_shellSurface, &QWaylandWlShellSurface::destroyed, this, &WlShellIntegration::handleShellSurfaceDestroyed);
}

void WlShellIntegration::handleStartMove(QWaylandSeat *seat)
{
    grabberState = GrabberState::Move;
    moveState.seat = seat;
    moveState.initialized = false;
}

void WlShellIntegration::handleStartResize(QWaylandSeat *seat, QWaylandWlShellSurface::ResizeEdge edges)
{
    grabberState = GrabberState::Resize;
    resizeState.seat = seat;
    resizeState.resizeEdges = edges;
    float scaleFactor = m_item->view()->output()->scaleFactor();
    resizeState.initialSize = m_shellSurface->surface()->size() / scaleFactor;
    resizeState.initialized = false;
}

void WlShellIntegration::handleSetDefaultTopLevel()
{
    if (isMaximized) {
        m_item->moveItem()->setPosition(maximizedState.initialPosition);
        isMaximized = false;
    }

    if (isFullScreen) {
        m_item->moveItem()->setPosition(fullScreenState.initialPosition);
        isFullScreen = false;
    }

    if (m_shellSurface->shell()->focusPolicy() == QWaylandShell::AutomaticFocus)
        m_item->takeFocus();
}

void WlShellIntegration::handleSetTransient(QWaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive)
{
    Q_UNUSED(parentSurface)
    Q_UNUSED(relativeToParent)

    if (m_shellSurface->shell()->focusPolicy() == QWaylandShell::AutomaticFocus && !inactive)
        m_item->takeFocus();
}

void WlShellIntegration::handleSetMaximized(QWaylandOutput *output)
{
    if (isMaximized)
        return;

    QWaylandOutput *designatedOutput = output ? output : m_item->findOutput();
    if (!designatedOutput)
        return;

    maximizedState.initialPosition = m_item->moveItem()->position();
    maximizedState.finalPosition = designatedOutput->position() + designatedOutput->availableGeometry().topLeft();
    isMaximized = true;

    auto scaleFactor = m_item->view()->output()->scaleFactor();
    m_shellSurface->sendConfigure(designatedOutput->availableGeometry().size() / scaleFactor, QWaylandWlShellSurface::NoneEdge);
}

void WlShellIntegration::handleSetFullScreen(QWaylandWlShellSurface::FullScreenMethod method, uint framerate, QWaylandOutput *output)
{
    Q_UNUSED(method);
    Q_UNUSED(framerate);

    if (isFullScreen)
        return;

    QWaylandOutput *designatedOutput = output ? output : m_item->findOutput();
    if (!designatedOutput)
        return;

    fullScreenState.initialPosition = m_item->moveItem()->position();
    fullScreenState.finalPosition = designatedOutput->position();
    isFullScreen = true;

    m_shellSurface->sendConfigure(designatedOutput->geometry().size(), QWaylandWlShellSurface::NoneEdge);
}

void WlShellIntegration::handleSetPopup(QWaylandSeat *seat, QWaylandSurface *parent, const QPoint &relativeToParent)
{
    Q_UNUSED(seat);

    // Find the parent item on the same output
    QWaylandQuickShellSurfaceItem *parentItem = nullptr;
    Q_FOREACH (QWaylandView *view, parent->views()) {
        if (view->output() == m_item->view()->output()) {
            QWaylandQuickShellSurfaceItem *item = qobject_cast<QWaylandQuickShellSurfaceItem*>(view->renderObject());
            if (item) {
                parentItem = item;
                break;
            }
        }
    }

    if (parentItem) {
        // Clear all the transforms for this ShellSurfaceItem. They are not
        // applicable when the item becomes a child to a surface that has its
        // own transforms. Otherwise the transforms would be applied twice.
        QQmlListProperty<QQuickTransform> t = m_item->transform();
        t.clear(&t);
        m_item->setRotation(0);
        m_item->setScale(1.0);
	auto scaleFactor = m_item->view()->output()->scaleFactor() / devicePixelRatio();
        m_item->setX(relativeToParent.x() * scaleFactor);
        m_item->setY(relativeToParent.y() * scaleFactor);
        m_item->setParentItem(parentItem);
    }

    isPopup = true;
    QWaylandQuickShellEventFilter::startFilter(m_shellSurface->surface()->client(), &closePopups);

    if (!popupShellSurfaces.contains(m_shellSurface)) {
        popupShellSurfaces.append(m_shellSurface);
        QObject::connect(m_shellSurface->surface(), &QWaylandSurface::mappedChanged,
                         this, &WlShellIntegration::handleSurfaceUnmapped);
    }
}

void WlShellIntegration::handlePopupClosed()
{
    handlePopupRemoved();
    if (m_shellSurface)
        QObject::disconnect(m_shellSurface->surface(), &QWaylandSurface::mappedChanged,
                            this, &WlShellIntegration::handleSurfaceUnmapped);
}

void WlShellIntegration::handlePopupRemoved()
{
    if (m_shellSurface)
        popupShellSurfaces.removeOne(m_shellSurface);
    if (popupShellSurfaces.isEmpty())
        QWaylandQuickShellEventFilter::cancelFilter();
    isPopup = false;
}


void WlShellIntegration::handleShellSurfaceDestroyed()
{
    if (isPopup)
        handlePopupRemoved();
    m_shellSurface = nullptr;
}

void WlShellIntegration::handleSurfaceUnmapped()
{
    if (!m_shellSurface || !m_shellSurface->surface()->size().isEmpty())
        return;
    handlePopupClosed();
}

void WlShellIntegration::redraw()
{
    if (isMaximized)
        m_item->moveItem()->setPosition(maximizedState.finalPosition);
    else if (isFullScreen)
        m_item->moveItem()->setPosition(fullScreenState.finalPosition);
}

void WlShellIntegration::adjustOffsetForNextFrame(const QPointF &offset)
{
    float scaleFactor = m_item->view()->output()->scaleFactor();
    QQuickItem *moveItem = m_item->moveItem();
    moveItem->setPosition(moveItem->position() + offset * scaleFactor / devicePixelRatio());
}

bool WlShellIntegration::mouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        Q_ASSERT(resizeState.seat == m_item->compositor()->seatFor(event));
        if (!resizeState.initialized) {
            resizeState.initialMousePos = event->windowPos();
            resizeState.initialized = true;
            return true;
        }
        float scaleFactor = m_item->view()->output()->scaleFactor();
        QPointF delta = (event->windowPos() - resizeState.initialMousePos) / scaleFactor * devicePixelRatio();
        QSize newSize = m_shellSurface->sizeForResize(resizeState.initialSize, delta, resizeState.resizeEdges);
        m_shellSurface->sendConfigure(newSize, resizeState.resizeEdges);
    } else if (grabberState == GrabberState::Move) {
        Q_ASSERT(moveState.seat == m_item->compositor()->seatFor(event));
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

bool WlShellIntegration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

QVector<QWaylandWlShellSurface*> WlShellIntegration::popupShellSurfaces;

void WlShellIntegration::closePopups()
{
    if (!popupShellSurfaces.isEmpty()) {
        Q_FOREACH (QWaylandWlShellSurface* shellSurface, popupShellSurfaces) {
            shellSurface->sendPopupDone();
        }
        popupShellSurfaces.clear();
    }
}

qreal WlShellIntegration::devicePixelRatio() const
{
    return m_item->window() ? m_item->window()->devicePixelRatio() : 1;
}

}

QT_END_NAMESPACE
