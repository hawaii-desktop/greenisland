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

#ifndef GREENISLAND_INPUTDEVICE_H
#define GREENISLAND_INPUTDEVICE_H

#include <QtCore/qnamespace.h>
#include <QtCore/QPoint>
#include <QtCore/QString>

#include <greenisland/greenisland_export.h>

class QKeyEvent;
class QTouchEvent;
class QInputEvent;

namespace GreenIsland {

class AbstractCompositor;
class Surface;
class SurfaceView;
class WlInputDevice;

class GREENISLAND_EXPORT Keymap
{
public:
    Keymap(const QString &layout = QString(), const QString &variant = QString(), const QString &options = QString(),
                   const QString &model = QString(), const QString &rules = QString());

    inline QString layout() const { return m_layout; }
    inline QString variant() const { return m_variant; }
    inline QString options() const { return m_options; }
    inline QString rules() const { return m_rules; }
    inline QString model() const { return m_model; }

private:
    QString m_layout;
    QString m_variant;
    QString m_options;
    QString m_rules;
    QString m_model;
};

class GREENISLAND_EXPORT InputDevice
{
public:
    enum CapabilityFlag {
        // The order should match the enum WL_SEAT_CAPABILITY_*
        Pointer = 0x01,
        Keyboard = 0x02,
        Touch = 0x04,

        DefaultCapabilities = Pointer | Keyboard | Touch
    };
    Q_DECLARE_FLAGS(CapabilityFlags, CapabilityFlag)

    InputDevice(AbstractCompositor *compositor, CapabilityFlags caps = DefaultCapabilities);
    virtual ~InputDevice();

    void sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(SurfaceView *surface , const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendFullKeyEvent(QKeyEvent *event);
    void sendFullKeyEvent(Surface *surface, QKeyEvent *event);

    void sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFullTouchEvent(QTouchEvent *event);

    Surface *keyboardFocus() const;
    bool setKeyboardFocus(Surface *surface);
    void setKeymap(const Keymap &keymap);

    SurfaceView *mouseFocus() const;
    void setMouseFocus(SurfaceView *surface, const QPointF &local_pos, const QPointF &global_pos = QPointF());

    AbstractCompositor *compositor() const;
    WlInputDevice *handle() const;

    InputDevice::CapabilityFlags capabilities();

    virtual bool isOwner(QInputEvent *inputEvent);

private:
    WlInputDevice *d;
    Q_DISABLE_COPY(InputDevice)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(InputDevice::CapabilityFlags)

} // namespace GreenIsland

#endif // GREENISLAND_INPUTDEVICE_H
