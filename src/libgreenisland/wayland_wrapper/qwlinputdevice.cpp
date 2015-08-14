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

#include <QtGui/QTouchEvent>

#include "abstractcompositor.h"
#include "drag.h"
#include "surfaceview.h"

#include "qwlcompositor_p.h"
#include "qwldatadevice_p.h"
#include "qwlinputdevice_p.h"
#include "qwlinputmethod_p.h"
#include "qwlsurface_p.h"
#include "protocols/qt/qttouchextension.h"
#include "qwlqtkey_p.h"
#include "qwlpointer_p.h"
#include "qwlkeyboard_p.h"
#include "qwltouch_p.h"

namespace GreenIsland {

WlInputDevice::WlInputDevice(InputDevice *handle, WlCompositor *compositor, InputDevice::CapabilityFlags caps)
    : QtWaylandServer::wl_seat(compositor->wl_display(), 4)
    , m_handle(handle)
    , m_dragHandle(new Drag(this))
    , m_compositor(compositor)
    , m_capabilities(caps)
    , m_pointer(m_capabilities & InputDevice::Pointer ? new WlPointer(m_compositor, this) : 0)
    , m_keyboard(m_capabilities & InputDevice::Keyboard ? new WlKeyboard(m_compositor, this) : 0)
    , m_touch(m_capabilities & InputDevice::Touch ? new WlTouch(m_compositor) : 0)
    , m_inputMethod(m_compositor->extensions() & AbstractCompositor::TextInputExtension ? new WlInputMethod(m_compositor, this) : 0)
    , m_data_device()
{
}

WlInputDevice::~WlInputDevice()
{
}

WlPointer *WlInputDevice::pointerDevice()
{
    return m_pointer.data();
}

WlKeyboard *WlInputDevice::keyboardDevice()
{
    return m_keyboard.data();
}

WlTouch *WlInputDevice::touchDevice()
{
    return m_touch.data();
}

WlInputMethod *WlInputDevice::inputMethod()
{
    return m_inputMethod.data();
}

const WlPointer *WlInputDevice::pointerDevice() const
{
    return m_pointer.data();
}

const WlKeyboard *WlInputDevice::keyboardDevice() const
{
    return m_keyboard.data();
}

const WlTouch *WlInputDevice::touchDevice() const
{
    return m_touch.data();
}

void WlInputDevice::seat_destroy_resource(wl_seat::Resource *)
{
//    cleanupDataDeviceForClient(resource->client(), true);
}

void WlInputDevice::seat_bind_resource(wl_seat::Resource *resource)
{
    // The order of m_capabilities matches the order defined in the wayland protocol
    wl_seat::send_capabilities(resource->handle, (uint32_t)m_capabilities);
}

void WlInputDevice::setCapabilities(InputDevice::CapabilityFlags caps)
{
    if (m_capabilities != caps) {
        InputDevice::CapabilityFlags changed = caps ^ m_capabilities;

        if (changed & InputDevice::Pointer) {
            m_pointer.reset(m_pointer.isNull() ? new WlPointer(m_compositor, this) : 0);
        }

        if (changed & InputDevice::Keyboard) {
            m_keyboard.reset(m_keyboard.isNull() ? new WlKeyboard(m_compositor, this) : 0);
        }

        if (changed & InputDevice::Touch) {
            m_touch.reset(m_touch.isNull() ? new WlTouch(m_compositor) : 0);
        }

        m_capabilities = caps;
        QList<Resource *> resources = resourceMap().values();
        for (int i = 0; i < resources.size(); i++) {
            wl_seat::send_capabilities(resources.at(i)->handle, (uint32_t)m_capabilities);
        }
    }
}

void WlInputDevice::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_pointer.isNull()) {
        m_pointer->add(resource->client(), id, resource->version());
    }
}

void WlInputDevice::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_keyboard.isNull()) {
        m_keyboard->add(resource->client(), id, resource->version());
    }
}

void WlInputDevice::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_touch.isNull()) {
        m_touch->add(resource->client(), id, resource->version());
    }
}

void WlInputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMousePressEvent(button, localPos, globalPos);
}

void WlInputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMouseReleaseEvent(button, localPos, globalPos);
}

void WlInputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMouseMoveEvent(localPos, globalPos);
}

void WlInputDevice::sendMouseMoveEvent(SurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    setMouseFocus(surface,localPos,globalPos);
    sendMouseMoveEvent(localPos,globalPos);
}

void WlInputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    pointerDevice()->sendMouseWheelEvent(orientation, delta);
}

void WlInputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    if (m_touch.isNull()) {
        return;
    }

    switch (state) {
    case Qt::TouchPointPressed:
        m_touch->sendDown(id, QPointF(x, y));
        break;
    case Qt::TouchPointMoved:
        m_touch->sendMotion(id, QPointF(x, y));
        break;
    case Qt::TouchPointReleased:
        m_touch->sendUp(id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
    default:
        break;
    }
}

void WlInputDevice::sendTouchFrameEvent()
{
    if (!m_touch.isNull()) {
        m_touch->sendFrame();
    }
}

void WlInputDevice::sendTouchCancelEvent()
{
    if (!m_touch.isNull()) {
        m_touch->sendCancel();
    }
}

void WlInputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

    QtKeyExtensionGlobal *ext = m_compositor->qtkeyExtension();
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()))
        return;

    if (!m_keyboard.isNull() && !event->isAutoRepeat()) {
        if (event->type() == QEvent::KeyPress)
            m_keyboard->sendKeyPressEvent(event->nativeScanCode());
        else if (event->type() == QEvent::KeyRelease)
            m_keyboard->sendKeyReleaseEvent(event->nativeScanCode());
    }
}

void WlInputDevice::sendFullKeyEvent(WlSurface *surface, QKeyEvent *event)
{
    QtKeyExtensionGlobal *ext = m_compositor->qtkeyExtension();
    if (ext)
        ext->postQtKeyEvent(event, surface);
}

void WlInputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    if (!mouseFocus()) {
        qWarning("Cannot send touch event, no pointer focus, fix the compositor");
        return;
    }

    if (event->type() == QEvent::TouchCancel) {
        sendTouchCancelEvent();
        return;
    }

    QtTouchExtension *ext = m_compositor->touchExtension();
    if (ext && ext->postTouchEvent(event, mouseFocus()))
        return;

    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    QPointF pos = mouseFocus()->pos();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        QPointF p = tp.pos() - pos;
        sendTouchPointEvent(tp.id(), p.x(), p.y(), tp.state());
    }
    sendTouchFrameEvent();
}

WlSurface *WlInputDevice::keyboardFocus() const
{
    return m_keyboard.isNull() ? 0 : m_keyboard->focus();
}

/*!
 * \return True if the keyboard focus is changed successfully. False for inactive transient surfaces.
 */
bool WlInputDevice::setKeyboardFocus(WlSurface *surface)
{
    if (surface && (surface->transientInactive() || surface->isDestroyed()))
        return false;

    if (!m_keyboard.isNull()) {
        m_keyboard->setFocus(surface);
        if (m_data_device)
            m_data_device->setFocus(m_keyboard->focusResource());
        return true;
    }
    return false;
}

SurfaceView *WlInputDevice::mouseFocus() const
{
    return m_pointer.isNull() ? 0 : m_pointer->focusSurface();
}

void WlInputDevice::setMouseFocus(SurfaceView *view, const QPointF &localPos, const QPointF &globalPos)
{
    if (view && view->surface()->handle()->isDestroyed())
        return;

    if (!m_pointer.isNull()) {
        m_pointer->setMouseFocus(view, localPos, globalPos);
    }

    if (!m_touch.isNull()) {
        // We have no separate touch focus management so make it match the pointer focus always.
        // No wl_touch_set_focus() is available so set it manually.
        m_touch->setFocus(view);
    }
}

void WlInputDevice::clientRequestedDataDevice(WlDataDeviceManager *, struct wl_client *client, uint32_t id)
{
    if (!m_data_device)
        m_data_device.reset(new WlDataDevice(this));
    m_data_device->add(client, id, 1);
}

WlCompositor *WlInputDevice::compositor() const
{
    return m_compositor;
}

InputDevice *WlInputDevice::handle() const
{
    return m_handle;
}

Drag *WlInputDevice::dragHandle() const
{
    return m_dragHandle.data();
}

const WlDataDevice *WlInputDevice::dataDevice() const
{
    return m_data_device.data();
}

} // namespace GreenIsland
