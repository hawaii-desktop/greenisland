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

#include "compositor.h"
#include "compositor_p.h"
#include "region.h"
#include "region_p.h"
#include "registry.h"
#include "registry_p.h"
#include "surface.h"
#include "surface_p.h"

namespace GreenIsland {

namespace Client {

/*
 * CompositorPrivate
 */

CompositorPrivate::CompositorPrivate()
    : QtWayland::wl_compositor()
{
}

/*
 * Compositor
 */

Compositor::Compositor(QObject *parent)
    : QObject(*new CompositorPrivate(), parent)
{
}

Surface *Compositor::createSurface(QObject *parent)
{
    Q_D(Compositor);

    if (!d->isInitialized())
        return Q_NULLPTR;

    Surface *surface = new Surface(parent);
    SurfacePrivate::get(surface)->init(d->create_surface());
    return surface;
}

Region *Compositor::createRegion(QObject *parent)
{
    return createRegion(QRegion(), parent);
}

Region *Compositor::createRegion(const QRegion &r, QObject *parent)
{
    Q_D(Compositor);

    if (!d->isInitialized())
        return Q_NULLPTR;

    Region *region = new Region(r, parent);
    RegionPrivate::get(region)->init(d->create_region());
    return region;
}

QByteArray Compositor::interfaceName()
{
    return QByteArrayLiteral("wl_compositor");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_compositor.cpp"
