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

#include "pointer.h"
#include "pointer_p.h"
#include "seat.h"
#include "surface.h"
#include "surface_p.h"

namespace GreenIsland {

namespace Client {

/*
 * PointerPrivate
 */

PointerPrivate::PointerPrivate()
    : QtWayland::wl_pointer()
    , seat(Q_NULLPTR)
    , seatVersion(0)
    , cursorSurface(Q_NULLPTR)
    , focusSurface(Q_NULLPTR)
    , enterSerial(0)
{
}

PointerPrivate::~PointerPrivate()
{
    if (seatVersion >= 3)
        release();
}

Pointer *PointerPrivate::fromWlPointer(struct ::wl_pointer *pointer)
{
    QtWayland::wl_pointer *wlPointer =
            static_cast<QtWayland::wl_pointer *>(wl_pointer_get_user_data(pointer));
    return static_cast<PointerPrivate *>(wlPointer)->q_func();
}

void PointerPrivate::pointer_enter(uint32_t serial, struct ::wl_surface *surface,
                                   wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    Q_UNUSED(surface_x);
    Q_UNUSED(surface_y);

    Q_Q(Pointer);

    enterSerial = serial;
    focusSurface = SurfacePrivate::fromWlSurface(surface);
    Q_EMIT q->focusSurfaceChanged();
    Q_EMIT q->enter(serial, QPointF(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y)));
}

void PointerPrivate::pointer_leave(uint32_t serial, struct ::wl_surface *surface)
{
    Q_UNUSED(surface);

    Q_Q(Pointer);

    focusSurface = Q_NULLPTR;
    Q_EMIT q->focusSurfaceChanged();
    Q_EMIT q->leave(serial);
}

void PointerPrivate::pointer_motion(uint32_t time, wl_fixed_t surface_x,
                                    wl_fixed_t surface_y)
{
    Q_Q(Pointer);

    QPoint pos(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
    Q_EMIT q->motion(time, pos);
}

void PointerPrivate::pointer_button(uint32_t serial, uint32_t time, uint32_t button,
                                    uint32_t state)
{
    Q_Q(Pointer);

    Qt::MouseButton b;
    switch (button) {
    case 0x110: b = Qt::LeftButton; break;
    case 0x111: b = Qt::RightButton; break;
    case 0x112: b = Qt::MiddleButton; break;
    case 0x113: b = Qt::ExtraButton1; break;
    case 0x114: b = Qt::ExtraButton2; break;
    case 0x115: b = Qt::ExtraButton3; break;
    case 0x116: b = Qt::ExtraButton4; break;
    case 0x117: b = Qt::ExtraButton5; break;
    case 0x118: b = Qt::ExtraButton6; break;
    case 0x119: b = Qt::ExtraButton7; break;
    case 0x11a: b = Qt::ExtraButton8; break;
    case 0x11b: b = Qt::ExtraButton9; break;
    case 0x11c: b = Qt::ExtraButton10; break;
    case 0x11d: b = Qt::ExtraButton11; break;
    case 0x11e: b = Qt::ExtraButton12; break;
    case 0x11f: b = Qt::ExtraButton13; break;
    default: return;
    }

    if (state == QtWayland::wl_pointer::button_state_pressed)
        Q_EMIT q->buttonPressed(serial, time, b);
    else
        Q_EMIT q->buttonReleased(serial, time, b);
}

void PointerPrivate::pointer_axis(uint32_t time, uint32_t axis, wl_fixed_t value)
{
    Q_Q(Pointer);

    Qt::Orientation orientation =
            axis == QtWayland::wl_pointer::axis_horizontal_scroll
            ? Qt::Horizontal : Qt::Vertical;
    Q_EMIT q->axisChanged(time, orientation, wl_fixed_to_double(value));
}

/*
 * Pointer
 */

Pointer::Pointer(Seat *seat)
    : QObject(*new PointerPrivate(), seat)
{
    qRegisterMetaType<Qt::MouseButton>("Qt::MouseButton");

    d_func()->seat = seat;
    d_func()->seatVersion = seat->version();
}

Surface *Pointer::focusSurface() const
{
    Q_D(const Pointer);
    return d->focusSurface;
}

Surface *Pointer::cursorSurface() const
{
    Q_D(const Pointer);
    return d->cursorSurface;
}

void Pointer::setCursor(Surface *surface, const QPoint &hotSpot)
{
    Q_D(Pointer);

    d->set_cursor(d->enterSerial, SurfacePrivate::get(surface)->object(),
                  hotSpot.x(), hotSpot.y());
}

QByteArray Pointer::interfaceName()
{
    return QByteArrayLiteral("wl_pointer");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_pointer.cpp"
