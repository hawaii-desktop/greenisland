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

#ifndef WLINPUTDEVICE_H
#define WLINPUTDEVICE_H

#include <stdint.h>

#include <greenisland/greenisland_export.h>
#include "inputdevice.h"

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QScopedPointer>

#ifdef HAVE_XKBCOMMON
#include <xkbcommon/xkbcommon.h>
#endif

#include "qwayland-server-wayland.h"

class QKeyEvent;
class QTouchEvent;

namespace GreenIsland {

class InputDevice;
class Drag;
class SurfaceView;

class WlCompositor;
class WlDataDevice;
class WlSurface;
class WlDataDeviceManager;
class WlPointer;
class WlKeyboard;
class WlTouch;
class WlInputMethod;

class GREENISLAND_EXPORT WlInputDevice : public QtWaylandServer::wl_seat
{
public:
    WlInputDevice(InputDevice *handle, WlCompositor *compositor, InputDevice::CapabilityFlags caps);
    ~WlInputDevice();

    void sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(SurfaceView *surface, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    void sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFullKeyEvent(QKeyEvent *event);
    void sendFullKeyEvent(WlSurface *surface, QKeyEvent *event);

    void sendFullTouchEvent(QTouchEvent *event);

    WlSurface *keyboardFocus() const;
    bool setKeyboardFocus(WlSurface *surface);

    SurfaceView *mouseFocus() const;
    void setMouseFocus(SurfaceView *surface, const QPointF &localPos, const QPointF &globalPos);

    void clientRequestedDataDevice(WlDataDeviceManager *dndSelection, struct wl_client *client, uint32_t id);
    const WlDataDevice *dataDevice() const;

    WlCompositor *compositor() const;
    InputDevice *handle() const;
    Drag *dragHandle() const;

    WlPointer *pointerDevice();
    WlKeyboard *keyboardDevice();
    WlTouch *touchDevice();
    WlInputMethod *inputMethod();

    const WlPointer *pointerDevice() const;
    const WlKeyboard *keyboardDevice() const;
    const WlTouch *touchDevice() const;

    static WlInputDevice *fromSeatResource(struct ::wl_resource *resource)
    {
        return static_cast<WlInputDevice *>(wl_seat::Resource::fromResource(resource)->seat_object);
    }

    InputDevice::CapabilityFlags capabilities() { return m_capabilities; }
    void setCapabilities(InputDevice::CapabilityFlags caps);

private:
    InputDevice *m_handle;
    QScopedPointer<Drag> m_dragHandle;
    WlCompositor *m_compositor;
    InputDevice::CapabilityFlags m_capabilities;

    QScopedPointer<WlPointer> m_pointer;
    QScopedPointer<WlKeyboard> m_keyboard;
    QScopedPointer<WlTouch> m_touch;
    QScopedPointer<WlInputMethod> m_inputMethod;
    QScopedPointer<WlDataDevice> m_data_device;

    void seat_bind_resource(wl_seat::Resource *resource) Q_DECL_OVERRIDE;

    void seat_get_pointer(wl_seat::Resource *resource,
                          uint32_t id) Q_DECL_OVERRIDE;
    void seat_get_keyboard(wl_seat::Resource *resource,
                           uint32_t id) Q_DECL_OVERRIDE;
    void seat_get_touch(wl_seat::Resource *resource,
                        uint32_t id) Q_DECL_OVERRIDE;

    void seat_destroy_resource(wl_seat::Resource *resource) Q_DECL_OVERRIDE;
};

} // namespace GreenIsland

#endif // WLINPUTDEVICE_H
