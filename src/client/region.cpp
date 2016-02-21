/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
