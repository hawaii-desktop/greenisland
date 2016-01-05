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

#include "seat.h"
#include "surface.h"
#include "surface_p.h"
#include "touch.h"
#include "touch_p.h"

namespace GreenIsland {

namespace Client {

/*
 * TouchPrivate
 */

TouchPrivate::TouchPrivate()
    : QtWayland::wl_touch()
    , seat(Q_NULLPTR)
    , seatVersion(0)
    , active(false)
{
}

TouchPrivate::~TouchPrivate()
{
    if (seatVersion >= 3)
        release();
}

TouchPoint *TouchPrivate::getPressedPoint(qint32 id) const
{
    Q_FOREACH (TouchPoint *tp, points) {
        if (tp->id() == id && tp->isDown())
            return tp;
    }

    return Q_NULLPTR;
}

Touch *TouchPrivate::fromWlTouch(struct ::wl_touch *touch)
{
    QtWayland::wl_touch *wlTouch =
            static_cast<QtWayland::wl_touch *>(wl_touch_get_user_data(touch));
    return static_cast<TouchPrivate *>(wlTouch)->q_func();
}

void TouchPrivate::touch_down(uint32_t serial, uint32_t time, struct ::wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    Q_Q(Touch);

    TouchPoint *tp = new TouchPoint();
    tp->d_ptr->id = id;
    tp->d_ptr->downSerial = serial;
    tp->d_ptr->surface = SurfacePrivate::fromWlSurface(surface);
    tp->d_ptr->positions.append(QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
    tp->d_ptr->timestamps.append(time);

    if (active) {
        points.append(tp);
        Q_EMIT q->pointAdded(tp);
    } else {
        while (!points.isEmpty())
            delete points.takeLast();
        points.append(tp);
        active = true;
        Q_EMIT q->sequenceStarted(tp);
    }
}

void TouchPrivate::touch_up(uint32_t serial, uint32_t time, int32_t id)
{
    Q_Q(Touch);

    TouchPoint *tp = getPressedPoint(id);
    if (!tp)
        return;

    tp->d_ptr->upSerial = serial;
    tp->d_ptr->timestamps.append(time);
    tp->d_ptr->down = false;
    Q_EMIT q->pointRemoved(tp);

    // Sequence is finished when no touch point is down
    Q_FOREACH (TouchPoint *curPt, points) {
        if (curPt->isDown())
            return;
    }
    active = false;
    Q_EMIT q->sequenceFinished();
}

void TouchPrivate::touch_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    Q_Q(Touch);

    TouchPoint *tp = getPressedPoint(id);
    if (!tp)
        return;

    tp->d_ptr->positions.append(QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
    tp->d_ptr->timestamps.append(time);
    Q_EMIT q->pointMoved(tp);
}

void TouchPrivate::touch_frame()
{
    Q_Q(Touch);

    Q_EMIT q->frameFinished();
}

void TouchPrivate::touch_cancel()
{
    Q_Q(Touch);

    active = false;
    Q_EMIT q->sequenceCanceled();
}

TouchPointPrivate::TouchPointPrivate()
    : id(0)
    , upSerial(0)
    , downSerial(0)
    , down(true)
    , q_ptr(Q_NULLPTR)
{
}

/*
 * Touch
 */

Touch::Touch(Seat *seat)
    : QObject(*new TouchPrivate(), seat)
{
    d_func()->seat = seat;
    d_func()->seatVersion = seat->version();
}

QByteArray Touch::interfaceName()
{
    return QByteArrayLiteral("wl_touch");
}

TouchPoint::TouchPoint()
    : d_ptr(new TouchPointPrivate())
{
    d_ptr->q_ptr = this;
}

TouchPoint::~TouchPoint()
{
    delete d_ptr;
}

qint32 TouchPoint::id() const
{
    Q_D(const TouchPoint);
    return d->id;
}

quint32 TouchPoint::upSerial() const
{
    Q_D(const TouchPoint);
    return d->upSerial;
}

quint32 TouchPoint::downSerial() const
{
    Q_D(const TouchPoint);
    return d->downSerial;
}

quint32 TouchPoint::timestamp() const
{
    Q_D(const TouchPoint);

    if (d->timestamps.isEmpty())
        return 0;
    return d->timestamps.last();
}

QVector<quint32> TouchPoint::timestamps() const
{
    Q_D(const TouchPoint);
    return d->timestamps;
}

QPointF TouchPoint::position() const
{
    Q_D(const TouchPoint);

    if (d->positions.isEmpty())
        return QPointF();
    return d->positions.last();
}

QVector<QPointF> TouchPoint::positions() const
{
    Q_D(const TouchPoint);
    return d->positions;
}

QPointer<Surface> TouchPoint::surface() const
{
    Q_D(const TouchPoint);
    return d->surface;
}

bool TouchPoint::isDown() const
{
    Q_D(const TouchPoint);
    return d->down;
}

} // namespace Client

} // namespace GreenIsland

#include "moc_touch.cpp"
