/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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
