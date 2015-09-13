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

#ifndef GTKSURFACE_H
#define GTKSURFACE_H

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceInterface>

#include "gtkshell.h"

namespace GreenIsland {

class GtkSurface : public QObject, public QWaylandSurfaceInterface, public QtWaylandServer::gtk_surface
{
    Q_OBJECT
public:
    GtkSurface(GtkShell *shell, QWaylandSurface *surface,
               wl_client *client, uint32_t id, uint32_t version);
    ~GtkSurface();

    QString applicationId() { return m_applicationId; }

protected:
    bool runOperation(QWaylandSurfaceOp *op) Q_DECL_OVERRIDE;

    void surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void surface_set_dbus_properties(Resource *resource,
                                     const QString &application_id,
                                     const QString &app_menu_path,
                                     const QString &menubar_path,
                                     const QString &window_object_path,
                                     const QString &application_object_path,
                                     const QString &unique_bus_name) Q_DECL_OVERRIDE;

private:
    QString m_applicationId;
};

}

#endif // GTKSURFACE_H
