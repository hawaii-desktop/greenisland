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

#include "region.h"
#include "region_p.h"

namespace GreenIsland {

namespace Client {

/*
 * RegionPrivate
 */

RegionPrivate::RegionPrivate()
    : QtWayland::wl_region()
{
}

/*
 * Region
 */

Region::Region(const QRegion &region, QObject *parent)
    : QObject(*new RegionPrivate(), parent)
{
    d_func()->region = region;
}

bool Region::isInitialized() const
{
    Q_D(const Region);
    return d->isInitialized();
}

void Region::add(const QRegion &region)
{
    Q_D(Region);

    d->region = d->region.united(region);
    Q_FOREACH (const QRect &rect, region.rects())
        d->add(rect.x(), rect.y(), rect.width(), rect.height());
}

void Region::add(const QRect &rect)
{
    Q_D(Region);

    d->region = d->region.united(rect);
    d->add(rect.x(), rect.y(), rect.width(), rect.height());
}

void Region::subtract(const QRegion &region)
{
    Q_D(Region);

    d->region = d->region.subtracted(region);
    Q_FOREACH (const QRect &rect, region.rects())
        d->subtract(rect.x(), rect.y(), rect.width(), rect.height());
}

void Region::subtract(const QRect &rect)
{
    Q_D(Region);

    d->region = d->region.subtracted(rect);
    d->subtract(rect.x(), rect.y(), rect.width(), rect.height());
}

QByteArray Region::interfaceName()
{
    return QByteArray("wl_region");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_region.cpp"
