/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

#ifndef GREENISLAND_LIBINPUT_H
#define GREENISLAND_LIBINPUT_H

#include <QtCore/QObject>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

class QTouchDevice;

namespace GreenIsland {

namespace Platform {

class LibInputHandlerPrivate;

struct GREENISLANDPLATFORM_EXPORT LibInputKeyEvent
{
    int key;
    Qt::KeyboardModifiers modifiers;
    quint32 nativeScanCode;
    quint32 nativeVirtualKey;
    quint32 nativeModifiers;
    QString text;
    bool autoRepeat;
    ushort repeatCount;
};

struct GREENISLANDPLATFORM_EXPORT LibInputMouseEvent
{
    QPoint pos;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    int wheelDelta;
    Qt::Orientation wheelOrientation;
};

struct GREENISLANDPLATFORM_EXPORT LibInputTouchEvent
{
    QTouchDevice *device;
    QList<QWindowSystemInterface::TouchPoint> touchPoints;
    Qt::KeyboardModifiers modifiers;
};

class GREENISLANDPLATFORM_EXPORT LibInputHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool suspended READ isSuspended NOTIFY suspendedChanged)
    Q_DECLARE_PRIVATE(LibInputHandler)
public:
    enum CapabilityFlag {
        Pointer = 0x01,
        Keyboard = 0x02,
        Touch = 0x04,
        Gesture = 0x08
    };
    Q_DECLARE_FLAGS(Capabilities, CapabilityFlag)

    LibInputHandler(QObject *parent = 0);

    LibInputHandler::Capabilities capabilities() const;

    void setPointerPosition(const QPoint &pos);

    bool isSuspended() const;

public Q_SLOTS:
    void suspend();
    void resume();

Q_SIGNALS:
    void ready();
    void capabilitiesChanged();
    void suspendedChanged(bool);

    void keyboardCountChanged(int count);
    void pointerCountChanged(int count);
    void touchCountChanged(int count);
    void gestureCountChanged(int count);

    void touchDeviceRegistered(QTouchDevice *td);
    void touchDeviceUnregistered(QTouchDevice *td);

    void keyPressed(const LibInputKeyEvent &event);
    void keyReleased(const LibInputKeyEvent &event);

    void mousePressed(const LibInputMouseEvent &event);
    void mouseReleased(const LibInputMouseEvent &event);
    void mouseMoved(const LibInputMouseEvent &event);
    void mouseWheel(const LibInputMouseEvent &event);

    void touchEvent(const LibInputTouchEvent &event);
    void touchCancel(const LibInputTouchEvent &event);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_liEventHandler())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LibInputHandler::Capabilities)

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_LIBINPUT_H
