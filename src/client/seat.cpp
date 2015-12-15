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
 ***************************************************************************/

#include "pointer.h"
#include "pointer_p.h"
#include "seat.h"
#include "seat_p.h"

namespace GreenIsland {

namespace Client {

/*
 * SeatPrivate
 */

SeatPrivate::SeatPrivate()
    : QtWayland::wl_seat()
    , version(0)
    , compositor(Q_NULLPTR)
    , pointer(Q_NULLPTR)
{
}

void SeatPrivate::seat_capabilities(uint32_t capabilities)
{
    Q_Q(Seat);

    if (capabilities & capability_pointer && !pointer) {
        pointer = new Pointer(q);
        PointerPrivate::get(pointer)->init(get_pointer());
    } else if (!(capabilities & capability_pointer) && pointer) {
        delete pointer;
        pointer = Q_NULLPTR;
    }
}

/*
 * Seat
 */

Seat::Seat(Registry *registry, wl_compositor *compositor,
           quint32 name, quint32 version)
    : QObject(*new SeatPrivate(), registry)
{
    d_func()->version = version;
    d_func()->compositor = compositor;
    d_func()->init(registry->registry(), name, version);
}

quint32 Seat::version() const
{
    Q_D(const Seat);
    return d->version;
}

wl_compositor *Seat::compositor() const
{
    Q_D(const Seat);
    return d->compositor;
}

Pointer *Seat::pointer() const
{
    Q_D(const Seat);
    return d->pointer;
}

} // namespace Client

} // namespace GreenIsland

#include "moc_seat.cpp"
