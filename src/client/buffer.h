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

#ifndef GREENISLANDCLIENT_BUFFER_H
#define GREENISLANDCLIENT_BUFFER_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Shm>

namespace GreenIsland {

namespace Client {

class BufferPrivate;
class ShmPool;
class ShmPoolPrivate;

class GREENISLANDCLIENT_EXPORT Buffer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Buffer)
    Q_PROPERTY(uchar *address READ address CONSTANT)
    Q_PROPERTY(Shm::Format format READ format CONSTANT)
    Q_PROPERTY(QSize size READ size CONSTANT)
    Q_PROPERTY(quint32 stride READ stride CONSTANT)
    Q_PROPERTY(QImage image READ image CONSTANT)
    Q_PROPERTY(bool released READ isReleased WRITE setReleased NOTIFY releasedChanged)
    Q_PROPERTY(bool used READ isUsed WRITE setUsed NOTIFY usedChanged)
public:
    uchar *address() const;
    Shm::Format format() const;
    QSize size() const;
    qint32 stride() const;
    QImage image() const;

    bool isReleased() const;
    void setReleased(bool released);

    bool isUsed() const;
    void setUsed(bool used);

    void copy(const void *source);

Q_SIGNALS:
    void releasedChanged();
    void usedChanged();

private:
    explicit Buffer(ShmPool *pool, const QSize &size, qint32 stride,
                    qint32 offset, Shm::Format format);

    friend class ShmPoolPrivate;
};

typedef QWeakPointer<Buffer> BufferPtr;
typedef QSharedPointer<Buffer> BufferSharedPtr;

} // namespace Client

} //namespace GreenIsland

#endif // GREENISLANDCLIENT_BUFFER_H
