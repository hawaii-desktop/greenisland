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

#include <QtGui/QImage>

#include "buffer.h"
#include "buffer_p.h"

Q_DECLARE_METATYPE(uchar*)

namespace GreenIsland {

namespace Client {

/*
 * BufferPrivate
 */

BufferPrivate::BufferPrivate()
    : QtWayland::wl_buffer()
    , pool(Q_NULLPTR)
    , format(Shm::Format_ARGB32)
    , stride(0)
    , released(false)
    , used(false)
{
}

Buffer *BufferPrivate::fromWlBuffer(struct ::wl_buffer *buffer)
{
    QtWayland::wl_buffer *wlBuffer =
            static_cast<QtWayland::wl_buffer *>(wl_buffer_get_user_data(buffer));
    return static_cast<BufferPrivate *>(wlBuffer)->q_func();
}

/*
 * Buffer
 */

Buffer::Buffer(ShmPool *pool, const QSize &size, qint32 stride, qint32 offset, Shm::Format format)
    : QObject(*new BufferPrivate())
{
    d_func()->pool = pool;
    d_func()->size = size;
    d_func()->stride = stride;
    d_func()->offset = offset;
    d_func()->format = format;
}

uchar *Buffer::address() const
{
    Q_D(const Buffer);
    return reinterpret_cast<uchar *>(d->pool->address()) + d->offset;
}

Shm::Format Buffer::format() const
{
    Q_D(const Buffer);
    return d->format;
}

QSize Buffer::size() const
{
    Q_D(const Buffer);
    return d->size;
}

qint32 Buffer::stride() const
{
    Q_D(const Buffer);
    return d->stride;
}

QImage Buffer::image() const
{
    Q_D(const Buffer);
    QImage::Format f = d->format == Shm::Format_ARGB32
            ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    return QImage(address(), d->size.width(), d->size.height(), d->stride, f);
}

bool Buffer::isReleased() const
{
    Q_D(const Buffer);
    return d->released;
}

void Buffer::setReleased(bool released)
{
    Q_D(Buffer);

    if (d->released == released)
        return;

    d->released = released;
    Q_EMIT releasedChanged();
}

bool Buffer::isUsed() const
{
    Q_D(const Buffer);
    return d->used;
}

void Buffer::setUsed(bool used)
{
    Q_D(Buffer);

    if (d->used == used)
        return;

    d->used = used;
    Q_EMIT usedChanged();
}

void Buffer::copy(const void *source)
{
    Q_D(Buffer);
    memcpy(address(), source, d->size.height() * d->stride);
}

} // namespace Client

} //namespace GreenIsland

#include "moc_buffer.cpp"
