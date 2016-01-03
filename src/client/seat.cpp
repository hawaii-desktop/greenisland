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

#include "keyboard.h"
#include "keyboard_p.h"
#include "pointer.h"
#include "pointer_p.h"
#include "registry_p.h"
#include "seat.h"
#include "seat_p.h"
#include "touch.h"
#include "touch_p.h"

namespace GreenIsland {

namespace Client {

/*
 * SeatPrivate
 */

SeatPrivate::SeatPrivate()
    : QtWayland::wl_seat()
    , version(0)
    , keyboard(Q_NULLPTR)
    , pointer(Q_NULLPTR)
    , touch(Q_NULLPTR)
{
}

void SeatPrivate::seat_capabilities(uint32_t capabilities)
{
    Q_Q(Seat);

    if (capabilities & capability_keyboard && !keyboard) {
        keyboard = new Keyboard(q);
        KeyboardPrivate::get(keyboard)->init(get_keyboard());
        Q_EMIT q->keyboardAdded();
    } else if (!(capabilities & capability_keyboard) && keyboard) {
        delete keyboard;
        keyboard = Q_NULLPTR;
        Q_EMIT q->keyboardRemoved();
    }

    if (capabilities & capability_pointer && !pointer) {
        pointer = new Pointer(q);
        PointerPrivate::get(pointer)->init(get_pointer());
        Q_EMIT q->pointerAdded();
    } else if (!(capabilities & capability_pointer) && pointer) {
        delete pointer;
        pointer = Q_NULLPTR;
        Q_EMIT q->pointerRemoved();
    }

    if (capabilities & capability_touch && !touch) {
        touch = new Touch(q);
        TouchPrivate::get(touch)->init(get_touch());
        Q_EMIT q->touchAdded();
    } else if (!(capabilities & capability_touch) && touch) {
        delete touch;
        touch = Q_NULLPTR;
        Q_EMIT q->touchRemoved();
    }
}

void SeatPrivate::seat_name(const QString &name)
{
    Q_Q(Seat);

    if (this->name != name) {
        this->name = name;
        Q_EMIT q->nameChanged();
    }
}

/*
 * Seat
 */

Seat::Seat(QObject *parent)
    : QObject(*new SeatPrivate(), parent)
{
}

QString Seat::name() const
{
    Q_D(const Seat);
    return d->name;
}

quint32 Seat::version() const
{
    Q_D(const Seat);
    return d->version;
}

Keyboard *Seat::keyboard() const
{
    Q_D(const Seat);
    return d->keyboard;
}

Pointer *Seat::pointer() const
{
    Q_D(const Seat);
    return d->pointer;
}

Touch *Seat::touch() const
{
    Q_D(const Seat);
    return d->touch;
}

QByteArray Seat::interfaceName()
{
    return QByteArrayLiteral("wl_seat");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_seat.cpp"
