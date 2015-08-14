/****************************************************************************
 * This file is part of Green Island.
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2015 The Qt Company Ltd.
 *
 *     You may use this file under the terms of the BSD license as follows:
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in
 *         the documentation and/or other materials provided with the
 *         distribution.
 *       * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 *         of its contributors may be used to endorse or promote products derived
 *         from this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ***************************************************************************/

#include "inputdevice.h"

#include "qwlinputdevice_p.h"
#include "qwlkeyboard_p.h"
#include "abstractcompositor.h"
#include "qwlsurface_p.h"
#include "qwlcompositor_p.h"
#include "surfaceview.h"

namespace GreenIsland {

Keymap::Keymap(const QString &layout, const QString &variant, const QString &options, const QString &model, const QString &rules)
              : m_layout(layout)
              , m_variant(variant)
              , m_options(options)
              , m_rules(rules)
              , m_model(model)
{
}



InputDevice::InputDevice(AbstractCompositor *compositor, CapabilityFlags caps)
    : d(new WlInputDevice(this,compositor->handle(), caps))
{
}

InputDevice::~InputDevice()
{
    delete d;
}

void InputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMousePressEvent(button,localPos,globalPos);
}

void InputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseReleaseEvent(button,localPos,globalPos);
}

void InputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseMoveEvent(localPos,globalPos);
}

/** Convenience function that will set the mouse focus to the surface, then send the mouse move event.
 *  If the mouse focus is the same surface as the surface passed in, then only the move event is sent
 **/
void InputDevice::sendMouseMoveEvent(SurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseMoveEvent(surface,localPos,globalPos);
}

void InputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    d->sendMouseWheelEvent(orientation, delta);
}

void InputDevice::sendKeyPressEvent(uint code)
{
    d->keyboardDevice()->sendKeyPressEvent(code);
}

void InputDevice::sendKeyReleaseEvent(uint code)
{
    d->keyboardDevice()->sendKeyReleaseEvent(code);
}

void InputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    d->sendTouchPointEvent(id,x,y,state);
}

void InputDevice::sendTouchFrameEvent()
{
    d->sendTouchFrameEvent();
}

void InputDevice::sendTouchCancelEvent()
{
    d->sendTouchCancelEvent();
}

void InputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    d->sendFullTouchEvent(event);
}

void InputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    d->sendFullKeyEvent(event);
}

void InputDevice::sendFullKeyEvent(Surface *surface, QKeyEvent *event)
{
    d->sendFullKeyEvent(surface->handle(), event);
}

Surface *InputDevice::keyboardFocus() const
{
    WlSurface *wlsurface = d->keyboardFocus();
    if (wlsurface)
        return  wlsurface->waylandSurface();
    return 0;
}

bool InputDevice::setKeyboardFocus(Surface *surface)
{
    WlSurface *wlsurface = surface?surface->handle():0;
    return d->setKeyboardFocus(wlsurface);
}

void InputDevice::setKeymap(const Keymap &keymap)
{
    if (handle()->keyboardDevice())
        handle()->keyboardDevice()->setKeymap(keymap);
}

SurfaceView *InputDevice::mouseFocus() const
{
    return d->mouseFocus();
}

void InputDevice::setMouseFocus(SurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    d->setMouseFocus(surface,localPos,globalPos);
}

AbstractCompositor *InputDevice::compositor() const
{
    return d->compositor()->waylandCompositor();
}

WlInputDevice *InputDevice::handle() const
{
    return d;
}

InputDevice::CapabilityFlags InputDevice::capabilities()
{
    return d->capabilities();
}

bool InputDevice::isOwner(QInputEvent *inputEvent)
{
    Q_UNUSED(inputEvent);
    return true;
}

} // namespace GreenIsland
