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
