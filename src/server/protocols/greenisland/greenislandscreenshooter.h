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

#ifndef GREENISLANDSCREENSHOOTER_H
#define GREENISLANDSCREENSHOOTER_H

#include <QtCore/QLoggingCategory>
#include <QtCompositor/QWaylandGlobalInterface>

#include "qwayland-server-greenisland-screenshooter.h"

Q_DECLARE_LOGGING_CATEGORY(SCREENSHOOTER_PROTOCOL)

namespace GreenIsland {

class GreenIslandScreenshooterGlobal : public QObject, public QWaylandGlobalInterface
{
public:
    explicit GreenIslandScreenshooterGlobal(QObject *parent = 0);

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;
};

class GreenIslandScreenshooter : public QObject, public QtWaylandServer::greenisland_screenshooter
{
public:
    GreenIslandScreenshooter(wl_client *client, uint32_t name, uint32_t version, QObject *parent);
    ~GreenIslandScreenshooter();

protected:
    void screenshooter_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void screenshooter_shoot(Resource *resource,
                             wl_resource *outputResource,
                             wl_resource *bufferResource) Q_DECL_OVERRIDE;
};

}

#endif // GREENISLANDSCREENSHOOTER_H
