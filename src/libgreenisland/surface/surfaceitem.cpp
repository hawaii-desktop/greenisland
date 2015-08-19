/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "surfaceitem.h"
#include "quicksurface.h"
#include "abstractcompositor.h"
#include "inputdevice.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QQuickWindow>

#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

namespace GreenIsland {

QMutex *SurfaceItem::mutex = 0;

class SurfaceTextureProvider : public QSGTextureProvider
{
public:
    SurfaceTextureProvider() : t(0) { }

    QSGTexture *texture() const Q_DECL_OVERRIDE
    {
        if (t)
            t->setFiltering(smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return t;
    }

    bool smooth;
    QSGTexture *t;
};

SurfaceItem::SurfaceItem(QuickSurface *surface, QQuickItem *parent)
    : QQuickItem(parent)
    , SurfaceView(surface)
    , m_provider(0)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
    , m_resizeSurfaceToItem(false)
    , m_newTexture(false)

{
    if (!mutex)
        mutex = new QMutex;

    setFlag(ItemHasContents);

    update();

    if (surface) {
        setWidth(surface->size().width());
        setHeight(surface->size().height());
    }

    setSmooth(true);

    setAcceptedMouseButtons(acceptedMouseButtons() |
                            Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
                            Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
                            Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
                            Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
                            Qt::ExtraButton12 | Qt::ExtraButton13);
    setAcceptHoverEvents(true);
    if (surface) {
        connect(surface, &Surface::mapped, this, &SurfaceItem::surfaceMapped);
        connect(surface, &Surface::unmapped, this, &SurfaceItem::surfaceUnmapped);
        connect(surface, &Surface::surfaceDestroyed, this, &SurfaceItem::surfaceDestroyed);
        connect(surface, &Surface::parentChanged, this, &SurfaceItem::parentChanged);
        connect(surface, &Surface::sizeChanged, this, &SurfaceItem::updateSize);
        connect(surface, &Surface::configure, this, &SurfaceItem::updateBuffer);
        connect(surface, &Surface::redraw, this, &QQuickItem::update);
    }
    connect(this, &SurfaceItem::widthChanged, this, &SurfaceItem::updateSurfaceSize);
    connect(this, &SurfaceItem::heightChanged, this, &SurfaceItem::updateSurfaceSize);


    m_yInverted = surface ? surface->isYInverted() : true;
    emit yInvertedChanged();
}

SurfaceItem::~SurfaceItem()
{
    QMutexLocker locker(mutex);
    if (m_provider)
        m_provider->deleteLater();
}

bool SurfaceItem::isYInverted() const
{
    return m_yInverted;
}

QSGTextureProvider *SurfaceItem::textureProvider() const
{
    if (!m_provider)
        m_provider = new SurfaceTextureProvider();
    return m_provider;
}

void SurfaceItem::mousePressEvent(QMouseEvent *event)
{
    if (!surface())
        return;

    if (!surface()->inputRegionContains(event->pos())) {
        event->ignore();
        return;
    }

    InputDevice *inputDevice = compositor()->inputDeviceFor(event);
    if (inputDevice->mouseFocus() != this)
        inputDevice->setMouseFocus(this, event->localPos(), event->windowPos());
    inputDevice->sendMousePressEvent(event->button(), event->localPos(), event->windowPos());
}

void SurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (surface()) {
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->localPos(), event->windowPos());
    }
}

void SurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (surface()) {
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseReleaseEvent(event->button(), event->localPos(), event->windowPos());
    }
}

void SurfaceItem::hoverEnterEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->pos());
    }
}

void SurfaceItem::hoverMoveEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->pos());
    }
}

void SurfaceItem::wheelEvent(QWheelEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }

        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseWheelEvent(event->orientation(), event->delta());
    }
}

void SurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (surface()) {
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    }
}

void SurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (surface() && hasFocus()) {
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    }
}

void SurfaceItem::touchEvent(QTouchEvent *event)
{
    if (m_touchEventsEnabled) {
        InputDevice *inputDevice = compositor()->inputDeviceFor(event);

        if (event->type() == QEvent::TouchBegin) {
            QQuickItem *grabber = window()->mouseGrabberItem();
            if (grabber != this)
                grabMouse();
        }

        QPoint pointPos;
        const QList<QTouchEvent::TouchPoint> &points = event->touchPoints();
        if (!points.isEmpty())
            pointPos = points.at(0).pos().toPoint();

        if (event->type() == QEvent::TouchBegin && !surface()->inputRegionContains(pointPos)) {
            event->ignore();
            return;
        }

        event->accept();
        if (inputDevice->mouseFocus() != this) {
            inputDevice->setMouseFocus(this, pointPos, pointPos);
        }
        inputDevice->sendFullTouchEvent(event);
    } else {
        event->ignore();
    }
}

void SurfaceItem::mouseUngrabEvent()
{
    if (surface()) {
        QTouchEvent e(QEvent::TouchCancel);
        touchEvent(&e);
    }
}

void SurfaceItem::takeFocus(InputDevice *device)
{
    setFocus(true);

    if (!surface())
        return;

    InputDevice *target = device;
    if (!target) {
        target = compositor()->defaultInputDevice();
    }
    target->setKeyboardFocus(surface());
}

void SurfaceItem::surfaceMapped()
{
    update();
}

void SurfaceItem::surfaceUnmapped()
{
    update();
}

void SurfaceItem::parentChanged(Surface *newParent, Surface *oldParent)
{
    Q_UNUSED(oldParent);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

void SurfaceItem::updateSize()
{
    if (surface()) {
        setSize(surface()->size());
    }
}

void SurfaceItem::updateSurfaceSize()
{
    if (surface() && m_resizeSurfaceToItem) {
        surface()->requestSize(QSize(width(), height()));
    }
}

void SurfaceItem::setPos(const QPointF &pos)
{
    setPosition(pos);
}

QPointF SurfaceItem::pos() const
{
    return position();
}

void SurfaceItem::setParentView(SurfaceView *view)
{
    setParentItem(static_cast<SurfaceItem *>(view));
}

/*!
    \qmlproperty bool SurfaceItem::paintEnabled

    If this property is true, the \l item is hidden, though the texture
    will still be updated. As opposed to hiding the \l item by
    setting \l{Item::visible}{visible} to false, setting this property to true
    will not prevent mouse or keyboard input from reaching \l item.
*/
bool SurfaceItem::paintEnabled() const
{
    return m_paintEnabled;
}

void SurfaceItem::setPaintEnabled(bool enabled)
{
    m_paintEnabled = enabled;
    update();
}

void SurfaceItem::updateBuffer(bool hasBuffer)
{
    Q_UNUSED(hasBuffer)

    bool inv = m_yInverted;
    m_yInverted = surface()->isYInverted();
    if (inv != m_yInverted)
        emit yInvertedChanged();

    m_newTexture = true;
}

void SurfaceItem::updateTexture()
{
    updateTexture(false);
}

void SurfaceItem::updateTexture(bool changed)
{
    if (!m_provider)
        m_provider = new SurfaceTextureProvider();

    m_provider->t = static_cast<QuickSurface *>(surface())->texture();
    m_provider->smooth = smooth();
    if (m_newTexture || changed)
        emit m_provider->textureChanged();
    m_newTexture = false;
}

QSGNode *SurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    bool mapped = surface() && surface()->isMapped();

    if (!mapped || !m_provider || !m_provider->t || !m_paintEnabled) {
        delete oldNode;
        return 0;
    }

    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node)
        node = new QSGSimpleTextureNode();
    node->setTexture(m_provider->t);
    // Surface textures come by default with the OpenGL coordinate system, which is inverted relative
    // to the QtQuick one. So we're dealing with a double invertion here, and if isYInverted() returns
    // true it means it is NOT inverted relative to QtQuick, while if it returns false it means it IS.
    if (surface()->isYInverted()) {
        node->setRect(0, 0, width(), height());
    } else {
        node->setRect(0, height(), width(), -height());
    }

    return node;
}

void SurfaceItem::setTouchEventsEnabled(bool enabled)
{
    if (m_touchEventsEnabled != enabled) {
        m_touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

void SurfaceItem::setResizeSurfaceToItem(bool enabled)
{
    if (m_resizeSurfaceToItem != enabled) {
        m_resizeSurfaceToItem = enabled;
        emit resizeSurfaceToItemChanged();
    }
}

} // namespace GreenIsland
