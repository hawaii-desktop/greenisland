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

#include "shmpool.h"
#include "shmpool_p.h"

Q_LOGGING_CATEGORY(WLSHMPOOL, "greenisland.client.shmpool")

namespace GreenIsland {

namespace Client {

/*
 * ShmPoolPrivate
 */

ShmPoolPrivate::ShmPoolPrivate()
    : shm(Q_NULLPTR)
    , pool(Q_NULLPTR)
    , file(new QTemporaryFile())
    , data(Q_NULLPTR)
    , size(1024)
    , valid(false)
{
    file->setFileTemplate(QStringLiteral("greenisland-shm-XXXXXX"));
}

ShmPoolPrivate::~ShmPoolPrivate()
{
    release();
}

bool ShmPoolPrivate::create()
{
    if (!file->open()) {
        qCWarning(WLSHMPOOL) << "Cannot open temporary file for shm pool";
        return false;
    }

    if (::unlink(qPrintable(file->fileName())) != 0) {
        qCWarning(WLSHMPOOL) << "Cannot unlink temporary file for shm pool";
        return false;
    }

    if (::ftruncate(file->handle(), size) < 0) {
        qCWarning(WLSHMPOOL) << "Cannot set shm pool size";
        return false;
    }

    data = (uchar *)::mmap(Q_NULLPTR, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->handle(), 0);
    if (data == (uchar *)MAP_FAILED) {
        qCWarning(WLSHMPOOL, "Failed to mmap /dev/zero: %s", ::strerror(errno));
        data = Q_NULLPTR;
        return false;
    }

    pool = wl_shm_create_pool(shm, file->handle(), size);
    if (!pool) {
        qCWarning(WLSHMPOOL) << "Failed to create shm pool";
        ::munmap(data, size);
        data = Q_NULLPTR;
        return false;
    }

    return true;
}

bool ShmPoolPrivate::resize(size_t newSize)
{
    Q_Q(ShmPool);

    Q_ASSERT(data);
    Q_ASSERT(shm);
    Q_ASSERT(pool);
    Q_ASSERT(file->isOpen());

    if (::ftruncate(file->handle(), newSize) < 0) {
        qCWarning(WLSHMPOOL) << "Failed to resize shm pool";
        return false;
    }

    wl_shm_pool_resize(pool, newSize);

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

void ShmPoolPrivate::release()
{
    if (data) {
        ::munmap(data, size);
        data = Q_NULLPTR;
    }
    if (pool) {
        wl_shm_pool_destroy(pool);
        pool = Q_NULLPTR;
    }
    if (shm) {
        wl_shm_destroy(shm);
        shm = Q_NULLPTR;
    }
    file->close();
    valid = false;
}

/*
 * ShmPool
 */

ShmPool::ShmPool(wl_shm *shm, QObject *parent)
    : QObject(*new ShmPoolPrivate(), parent)
{
    d_func()->shm = shm;
    Q_ASSERT(shm);
    d_func()->valid = d_func()->create();
}

bool ShmPool::isValid() const
{
    Q_D(const ShmPool);
    return d->valid;
}

wl_shm *ShmPool::shm() const
{
    Q_D(const ShmPool);
    return d->shm;
}

} // namespace Client

} // namespace GreenIsland

#include "moc_shmpool.cpp"
