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

#ifndef GREENISLAND_REGION_P_H
#define GREENISLAND_REGION_P_H

#include <QtGui/QRegion>

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include "qwayland-server-wayland.h"

namespace GreenIsland {

class Region;

class RegionPrivate : public QtWaylandServer::wl_region
{
    Q_DECLARE_PUBLIC(Region)
public:
    RegionPrivate(wl_client *client, quint32 id, GreenIsland::Region *self);

    quint32 id() const;

    static RegionPrivate *fromResource(wl_resource *resource);

    QRegion region;

protected:
    Region *q_ptr;

private:
    void region_destroy_resource(Resource *) Q_DECL_OVERRIDE;

    void region_destroy(Resource *) Q_DECL_OVERRIDE;
    void region_add(Resource *resource,
                    int32_t x, int32_t y,
                    int32_t w, int32_t h) Q_DECL_OVERRIDE;
    void region_subtract(Resource *resource,
                         int32_t x, int32_t y,
                         int32_t w, int32_t h) Q_DECL_OVERRIDE;
};

} // namespace GreenIsland

#endif // GREENISLAND_REGION_P_H

