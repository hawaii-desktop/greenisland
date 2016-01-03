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

#ifndef GREENISLANDCLIENT_SEAT_H
#define GREENISLANDCLIENT_SEAT_H

#include <GreenIsland/Client/Registry>

namespace GreenIsland {

namespace Client {

class Keyboard;
class Pointer;
class SeatPrivate;
class Touch;

class GREENISLANDCLIENT_EXPORT Seat : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Seat)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
public:
    QString name() const;
    quint32 version() const;

    Keyboard *keyboard() const;
    Pointer *pointer() const;
    Touch *touch() const;

    static QByteArray interfaceName();

Q_SIGNALS:
    void nameChanged();
    void keyboardAdded();
    void keyboardRemoved();
    void pointerAdded();
    void pointerRemoved();
    void touchAdded();
    void touchRemoved();

private:
    Seat(QObject *parent = Q_NULLPTR);

    friend class Registry;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SEAT_H
