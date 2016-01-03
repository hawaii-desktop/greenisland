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

#include "buffer_p.h"
#include "shm_p.h"
#include "shmformats_p.h"
#include "shmpool.h"
#include "shmpool_p.h"

Q_LOGGING_CATEGORY(WLSHMPOOL, "greenisland.client.shmpool")

namespace GreenIsland {

namespace Client {

/*
 * ShmPoolPrivate
 */

ShmPoolPrivate::ShmPoolPrivate()
    : QtWayland::wl_shm_pool()
    , shm(Q_NULLPTR)
    , file(new QTemporaryFile())
    , data(Q_NULLPTR)
    , size(1024)
    , offset(0)
{
    file->setFileTemplate(QStringLiteral("greenisland-shm-XXXXXX"));
}

ShmPoolPrivate::~ShmPoolPrivate()
{
    buffers.clear();

    if (data) {
        ::munmap(data, size);
        data = Q_NULLPTR;
    }

    file->close();
}

bool ShmPoolPrivate::createPool(Shm *shm, size_t createSize)
{
    if (!file->open()) {
        qCWarning(WLSHMPOOL) << "Cannot open temporary file for shm pool";
        return false;
    }

    if (::unlink(qPrintable(file->fileName())) != 0) {
        qCWarning(WLSHMPOOL) << "Cannot unlink temporary file for shm pool";
        return false;
    }

    if (::ftruncate(file->handle(), createSize) < 0) {
        qCWarning(WLSHMPOOL) << "Cannot set shm pool size";
        return false;
    }

    data = (uchar *)::mmap(Q_NULLPTR, createSize, PROT_READ | PROT_WRITE, MAP_SHARED, file->handle(), 0);
    if (data == (uchar *)MAP_FAILED) {
        qCWarning(WLSHMPOOL, "Failed to mmap /dev/zero: %s", ::strerror(errno));
        data = Q_NULLPTR;
        return false;
    }

    ::wl_shm_pool *pool = ShmPrivate::get(shm)->create_pool(file->handle(), createSize);
    if (!pool) {
        qCWarning(WLSHMPOOL) << "Failed to create shm pool";
        ::munmap(data, size);
        data = Q_NULLPTR;
        return false;
    }

    init(pool);
    size = createSize;

    return true;
}

bool ShmPoolPrivate::resizePool(size_t newSize)
{
    Q_Q(ShmPool);

    Q_ASSERT(data);
    Q_ASSERT(file->isOpen());

    if (::ftruncate(file->handle(), newSize) < 0) {
        qCWarning(WLSHMPOOL) << "Failed to resize shm pool";
        return false;
    }

    resize(newSize);

    ::munmap(data, size);
    data = (uchar *)::mmap(Q_NULLPTR, newSize, PROT_READ | PROT_WRITE, MAP_SHARED, file->handle(), 0);
    if (data == (uchar *)MAP_FAILED) {
        qCWarning(WLSHMPOOL, "Failed to mmap /dev/zero: %s", ::strerror(errno));
        data = Q_NULLPTR;
        return false;
    }

    size = newSize;
    Q_EMIT q->resized();

    return true;
}

QVector<BufferSharedPtr>::iterator ShmPoolPrivate::reuseBuffer(const QSize &s,
                                                               qint32 stride,
                                                               Shm::Format format)
{
    Q_Q(ShmPool);

    // We can't return a shared pointer here so we just return
    // the iterator of the buffers vector that can be used to
    // retrieve the actual shared pointer

    // Try to reuse an existing buffer
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
        QSharedPointer<Buffer> buffer = (*it);

        // Skip buffers that are in use or do not match what the user is looking for
        if (!buffer->isReleased() || buffer->isUsed())
            continue;
        if (buffer->size() != s || buffer->stride() != stride || buffer->format() != format)
            continue;

        // We found the buffer
        buffer->setReleased(false);
        return it;
    }

    // No buffer found: resize the pool to accomodate a new buffer
    const qint32 bytesCount = s.height() * stride;
    if (offset + bytesCount > size) {
        if (!resizePool(bytesCount + size))
            return Q_NULLPTR;
    }

    // No buffer can be reused, create a new one and advance the offset
    wl_buffer *nativeBuffer =
            create_buffer(offset, s.width(), s.height(), stride, format);
    if (!nativeBuffer)
        return Q_NULLPTR;
    Buffer *buffer = new Buffer(q, s, stride, offset, format);
    BufferPrivate::get(buffer)->init(nativeBuffer);
    offset += bytesCount;
    return buffers.insert(buffers.end(), BufferSharedPtr(buffer));
}

/*
 * ShmPool
 */

ShmPool::ShmPool(Shm *shm)
    : QObject(*new ShmPoolPrivate(), shm)
{
    d_func()->shm = shm;
}

Shm *ShmPool::shm() const
{
    Q_D(const ShmPool);
    return d->shm;
}

void *ShmPool::address() const
{
    Q_D(const ShmPool);
    return d->data;
}

BufferPtr ShmPool::createBuffer(const QImage &image)
{
    Q_D(ShmPool);

    if (image.isNull())
        return BufferPtr();

    auto it = d->reuseBuffer(image.size(), image.bytesPerLine(),
                             static_cast<Shm::Format>(ShmFormats::fromQt(image.format())));
    if (it == d->buffers.end())
        return BufferPtr();
    (*it)->copy(image.bits());
    return BufferPtr(*it);
}

BufferPtr ShmPool::createBuffer(const QSize &size, quint32 stride, const void *source, Shm::Format format)
{
    Q_D(ShmPool);

    if (size.isEmpty())
        return BufferPtr();

    auto it = d->reuseBuffer(size, stride, format);
    if (it == d->buffers.end())
        return BufferPtr();
    if (source)
        (*it)->copy(source);
    return BufferPtr(*it);
}

BufferPtr ShmPool::findBuffer(const QSize &size, quint32 stride, Shm::Format format)
{
    Q_D(ShmPool);

    auto it = d->reuseBuffer(size, stride, format);
    if (it == d->buffers.end())
        return BufferPtr();
    return BufferPtr(*it);
}

QByteArray ShmPool::interfaceName()
{
    return QByteArrayLiteral("wl_shm_pool");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_shmpool.cpp"
