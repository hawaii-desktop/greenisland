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

#ifndef GREENISLANDCLIENT_SEAT_H
#define GREENISLANDCLIENT_SEAT_H

#include <GreenIsland/Client/Registry>

namespace GreenIsland {

namespace Client {

class Pointer;
class SeatPrivate;

class GREENISLANDCLIENT_EXPORT Seat : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Seat)
public:
    Seat(Registry *registry, wl_compositor *compositor,
         quint32 name, quint32 version);

    quint32 version() const;

    wl_compositor *compositor() const;
    Pointer *pointer() const;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SEAT_H
