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

#include "region.h"
#include "region_p.h"

namespace GreenIsland {

/*
 * RegionPrivate
 */

RegionPrivate::RegionPrivate(wl_client *client, quint32 id, Region *self)
    : QtWaylandServer::wl_region(client, id, 1)
    , q_ptr(self)
{
}

quint32 RegionPrivate::id() const
{
    return wl_resource_get_id(resource()->handle);
}

RegionPrivate *RegionPrivate::fromResource(wl_resource *resource)
{
    return static_cast<RegionPrivate *>(Resource::fromResource(resource)->region_object);
}

void RegionPrivate::region_destroy_resource(Resource *)
{
    delete this;
}

void RegionPrivate::region_destroy(Resource *)
{
    // Delete the public object, this will destroy the resource triggering
    // region_destroy_resource() which will delete the private object
    delete q_ptr;
}

void RegionPrivate::region_add(Resource *resource,
                               int32_t x, int32_t y,
                               int32_t w, int32_t h)
{
    Q_UNUSED(resource);
    region += QRect(x, y, w, h);
}

void RegionPrivate::region_subtract(Resource *resource,
                                    int32_t x, int32_t y,
                                    int32_t w, int32_t h)
{
    Q_UNUSED(resource);
    region -= QRect(x, y, w, h);
}

/*
 * Region
 */

Region::Region(wl_client *client, quint32 id)
    : d_ptr(new RegionPrivate(client, id, this))
{
}

Region::~Region()
{
    wl_resource_destroy(d_ptr->resource()->handle);
}

QRegion Region::region() const
{
    Q_D(const Region);
    return d->region;
}

Region *Region::fromResource(wl_resource *resource)
{
    RegionPrivate *d = RegionPrivate::fromResource(resource);
    return d->q_ptr;
}

} // namespace GreenIsland
