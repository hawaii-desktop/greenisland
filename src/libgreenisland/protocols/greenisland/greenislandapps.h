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

#ifndef GREENISLANDAPPS_H
#define GREENISLANDAPPS_H

#include "globalinterface.h"

#include "qwayland-server-greenisland.h"

namespace GreenIsland {

class GreenIslandApps : public GlobalInterface, public QtWaylandServer::greenisland_applications
{
public:
    GreenIslandApps();

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;

private:
    Resource *m_boundResource;

    void applications_bind_resource(Resource *resource) Q_DECL_OVERRIDE;

    void applications_quit(Resource *resource, const QString &app_id) Q_DECL_OVERRIDE;
};

}

#endif // GREENISLANDAPPS_H
