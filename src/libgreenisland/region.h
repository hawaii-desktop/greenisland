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

#ifndef GREENISLAND_REGION_H
#define GREENISLAND_REGION_H

#include <QtGui/QRegion>

#include <greenisland/greenisland_export.h>

struct wl_client;
struct wl_resource;

namespace GreenIsland {
class WlCompositor;
}

namespace GreenIsland {

class RegionPrivate;

class GREENISLAND_EXPORT Region
{
    Q_DISABLE_COPY(Region)
    Q_DECLARE_PRIVATE(Region)
public:
    QRegion region() const;

    static Region *fromResource(wl_resource *resource);

private:
    explicit Region(wl_client *client, quint32 id);

    RegionPrivate *const d_ptr;

    friend class GreenIsland::WlCompositor;
};

} // namespace GreenIsland

#endif // GREENISLAND_REGION_H
