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

#include "shm.h"
#include "shm_p.h"
#include "shmpool.h"
#include "shmpool_p.h"

namespace GreenIsland {

namespace Client {

/*
 * ShmPrivate
 */

ShmPrivate::ShmPrivate()
    : QtWayland::wl_shm()
{
}

void ShmPrivate::shm_format(uint32_t format)
{
    Q_Q(Shm);

    Shm::Format f = static_cast<Shm::Format>(format);
    formats.append(f);
    Q_EMIT q->formatAdded(f);
}

/*
 * Shm
 */

Shm::Shm(QObject *parent)
    : QObject(*new ShmPrivate(), parent)
{
}

QList<Shm::Format> Shm::formats() const
{
    Q_D(const Shm);
    return d->formats.toList();
}

ShmPool *Shm::createPool(size_t size)
{
    Q_D(Shm);
    ShmPool *pool = new ShmPool(this);
    ShmPoolPrivate *dPool = ShmPoolPrivate::get(pool);
    if (!dPool->createPool(this, size)) {
        delete pool;
        return Q_NULLPTR;
    }
    return pool;
}

QByteArray Shm::interfaceName()
{
    return QByteArrayLiteral("wl_shm");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_shm.cpp"
