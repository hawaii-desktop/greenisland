/****************************************************************************
 * This file is part of Hawaii.
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

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <GreenIsland/Compositor/QWaylandGlobalInterface>

#include "qwayland-server-plasma-shell.h"

Q_DECLARE_LOGGING_CATEGORY(PLASMA_SHELL_PROTOCOL)

namespace GreenIsland {

class Compositor;
class PlasmaSurface;

class PlasmaShell : public QObject, public QWaylandGlobalInterface, public QtWaylandServer::org_kde_plasma_shell
{
    Q_OBJECT
public:
    PlasmaShell(Compositor *compositor);
    ~PlasmaShell();

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;

    QList<PlasmaSurface *> surfaces() const;

private:
    Compositor *m_compositor;
    QList<PlasmaSurface *> m_surfaces;

    void shell_get_surface(Resource *resource, uint32_t id,
                           wl_resource *surfaceResource) Q_DECL_OVERRIDE;
    //void shell_lock(Resource *resource) Q_DECL_OVERRIDE;
    //void shell_unlock(Resource *resource) Q_DECL_OVERRIDE;
    void shell_desktop_ready(Resource *resource) Q_DECL_OVERRIDE;
    //void shell_present(Resource *resource, wl_array *surfaces) Q_DECL_OVERRIDE;
};

}

#endif // PLASMASHELL_H
