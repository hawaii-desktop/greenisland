/****************************************************************************
**
** Copyright (C) 2015-2016 Pier Luigi Fiorini
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include "logging.h"
#include "libinput/libinputhandler.h"
#include "libinput/libinputtouch.h"

#include <libinput.h>

namespace GreenIsland {

namespace Platform {

/*
 * LibInputTouchPrivate
 */

struct State {
    State()
        : touchDevice(Q_NULLPTR)
    {
    }

    QWindowSystemInterface::TouchPoint *touchPointAt(qint32 slot)
    {
        const int id = qMax(0, slot);

        for (int i = 0; i < touchPoints.size(); i++) {
            if (touchPoints.at(i).id == id)
                return &touchPoints[i];
        }

        return Q_NULLPTR;
    }

    QTouchDevice *touchDevice;
    QList<QWindowSystemInterface::TouchPoint> touchPoints;
};

class LibInputTouchPrivate
{
public:
    LibInputTouchPrivate(LibInputHandler *h)
        : handler(h)
    {
    }

    State *stateFromEvent(libinput_event_touch *e)
    {
        libinput_event *event = libinput_event_touch_get_base_event(e);
        libinput_device *device = libinput_event_get_device(event);
        return &state[device];
    }

    QPointF positionFromEvent(libinput_event_touch *e)
    {
        // Constrain size to the virtual desktop
        const QSize size = QGuiApplication::primaryScreen()->virtualGeometry().size();
        const double x = libinput_event_touch_get_x_transformed(e, size.width());
        const double y = libinput_event_touch_get_y_transformed(e, size.width());
        return QPointF(x, y);
    }

    LibInputHandler *handler;
    QHash<libinput_device *, State> state;
};

/*
 * LibInputTouch
 */

LibInputTouch::LibInputTouch(LibInputHandler *handler)
    : d_ptr(new LibInputTouchPrivate(handler))
{
}

LibInputTouch::~LibInputTouch()
{
    delete d_ptr;
}

QTouchDevice *LibInputTouch::registerDevice(libinput_device *device)
{
    Q_D(LibInputTouch);

    QTouchDevice *td = new QTouchDevice;
    td->setType(QTouchDevice::TouchScreen);
    td->setCapabilities(QTouchDevice::Position | QTouchDevice::Area);
    td->setName(QString::fromUtf8(libinput_device_get_name(device)));
    d->state[device].touchDevice = td;

    return td;
}

void LibInputTouch::unregisterDevice(libinput_device *device, QTouchDevice **td)
{
    Q_D(LibInputTouch);

    if (td && *td)
        *td = d->state[device].touchDevice;
}

void LibInputTouch::handleTouchUp(libinput_event_touch *event)
{
    Q_D(LibInputTouch);

    State *state = d->stateFromEvent(event);

    int slot = libinput_event_touch_get_slot(event);
    QWindowSystemInterface::TouchPoint *touchPoint = state->touchPointAt(slot);
    if (touchPoint) {
        touchPoint->state = Qt::TouchPointReleased;

        Qt::TouchPointStates states = 0;
        Q_FOREACH (const QWindowSystemInterface::TouchPoint &tp, state->touchPoints)
            states |= tp.state;
        if (states == Qt::TouchPointReleased)
            handleTouchFrame(event);
    } else {
        qCWarning(lcInput) << "Received a touch up without prior touch down for slot" << slot;
    }
}

void LibInputTouch::handleTouchDown(libinput_event_touch *event)
{
    Q_D(LibInputTouch);

    State *state = d->stateFromEvent(event);

    int slot = libinput_event_touch_get_slot(event);
    QWindowSystemInterface::TouchPoint *touchPoint = state->touchPointAt(slot);
    if (touchPoint) {
        // There shouldn't be already a touch point
        qCWarning(lcInput) << "Touch point already present for slot" << slot;
    } else {
        QWindowSystemInterface::TouchPoint newTouchPoint;
        newTouchPoint.id = qMax(0, slot);
        newTouchPoint.state = Qt::TouchPointPressed;
        newTouchPoint.area = QRect(0, 0, 8, 8);
        newTouchPoint.area.moveCenter(d->positionFromEvent(event));
        state->touchPoints.append(newTouchPoint);
    }
}

void LibInputTouch::handleTouchMotion(libinput_event_touch *event)
{
    Q_D(LibInputTouch);

    State *state = d->stateFromEvent(event);

    int slot = libinput_event_touch_get_slot(event);
    QWindowSystemInterface::TouchPoint *touchPoint = state->touchPointAt(slot);
    if (touchPoint) {
        const QPointF pos = d->positionFromEvent(event);
        if (touchPoint->area.center() != pos) {
            touchPoint->area.moveCenter(pos);

            if (touchPoint->state != Qt::TouchPointPressed)
                touchPoint->state = Qt::TouchPointMoved;
        } else {
            touchPoint->state = Qt::TouchPointStationary;
        }
    } else {
        qCWarning(lcInput) << "Received a touch motion without prior touch down for slot" << slot;
    }
}

void LibInputTouch::handleTouchCancel(libinput_event_touch *event)
{
    Q_D(LibInputTouch);

    State *state = d->stateFromEvent(event);
    if (state->touchDevice) {
        LibInputTouchEvent e;
        e.device = state->touchDevice;
        e.touchPoints = state->touchPoints;
        e.modifiers = QGuiApplication::keyboardModifiers();
        Q_EMIT d->handler->touchCancel(e);
    } else {
        qCWarning(lcInput) << "Received a touch canceled without a device";
    }
}

void LibInputTouch::handleTouchFrame(libinput_event_touch *event)
{
    Q_D(LibInputTouch);

    State *state = d->stateFromEvent(event);
    if (!state->touchDevice) {
        qCWarning(lcInput) << "Received a touch frame without a device";
        return;
    }

    if (state->touchPoints.isEmpty())
        return;

    LibInputTouchEvent e;
    e.device = state->touchDevice;
    e.touchPoints = state->touchPoints;
    e.modifiers = QGuiApplication::keyboardModifiers();
    Q_EMIT d->handler->touchEvent(e);

    for (int i = 0; i < state->touchPoints.size(); i++) {
        QWindowSystemInterface::TouchPoint &tp(state->touchPoints[i]);
        if (tp.state == Qt::TouchPointReleased)
            state->touchPoints.removeAt(i--);
        else if (tp.state == Qt::TouchPointPressed)
            tp.state = Qt::TouchPointStationary;
    }
}

} // namespace Platform

} // namespace GreenIsland
