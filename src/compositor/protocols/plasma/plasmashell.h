/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef PLASMASHELL_H
#define PLASMASHELL_H

#include <QtCore/QHash>
#include <QtCompositor/QWaylandGlobalInterface>

#include "qwayland-server-plasma-shell.h"

class PlasmaShell : public QWaylandGlobalInterface, public QtWaylandServer::org_kde_plasma_shell
{
public:
    explicit PlasmaShell();

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;

private:
    void shell_set_position(Resource *resource, wl_resource *outputResource,
                            wl_resource *surfaceResource,
                            int32_t x, int32_t y) Q_DECL_OVERRIDE;
    //void shell_set_screen_edge(Resource *resource, wl_resource *surface, uint32_t screen_edge) Q_DECL_OVERRIDE;
    //void shell_set_grab_surface(Resource *resource, wl_resource *surface) Q_DECL_OVERRIDE;
    void shell_set_surface_role(Resource *resource, wl_resource *output,
                                wl_resource *surfaceResource,
                                uint32_t role) Q_DECL_OVERRIDE;
    //void shell_lock(Resource *resource) Q_DECL_OVERRIDE;
    //void shell_unlock(Resource *resource) Q_DECL_OVERRIDE;
    void shell_desktop_ready(Resource *resource) Q_DECL_OVERRIDE;
    //void shell_present(Resource *resource, wl_array *surfaces) Q_DECL_OVERRIDE;
};

#endif // PLASMASHELL_H
