/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 * Copyright (C) 2013 Martin Gräßlin
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *    Martin Gräßlin <mgraesslin@kde.org>
 *
 * $BEGIN_LICENSE:LGPLKDE$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) version 3, or any later version accepted by the membership
 * of KDE e.V. (or its successor approved by the membership of KDE e.V.),
 * which shall act as a proxy defined in Section 6 of version 3 of the license.
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

#ifndef GREENISLANDCLIENT_SHMPOOL_H
#define GREENISLANDCLIENT_SHMPOOL_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/Shm>

namespace GreenIsland {

namespace Client {

class ShmPoolPrivate;

class GREENISLANDCLIENT_EXPORT ShmPool : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ShmPool)
public:
    Shm *shm() const;

    void *address() const;

    BufferPtr createBuffer(const QImage &image);
    BufferPtr createBuffer(const QSize &size, quint32 stride,
                           const void *source = Q_NULLPTR,
                           Shm::Format format = Shm::Format_ARGB32);

    BufferPtr findBuffer(const QSize &size, quint32 stride,
                         Shm::Format format = Shm::Format_ARGB32);

    static QByteArray interfaceName();

Q_SIGNALS:
    void resized();

private:
    ShmPool(Shm *shm);

    friend class Shm;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SHMPOOL_H
