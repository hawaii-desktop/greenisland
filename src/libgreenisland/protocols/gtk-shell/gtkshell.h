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

#ifndef GTKSHELL_H
#define GTKSHELL_H

#include <QtCore/QLoggingCategory>
#include <QtCompositor/QWaylandGlobalInterface>

#include "qwayland-server-gtk.h"

Q_DECLARE_LOGGING_CATEGORY(GTKSHELL_PROTOCOL)
Q_DECLARE_LOGGING_CATEGORY(GTKSHELL_TRACE)

namespace GreenIsland {

class GtkShellGlobal : public QObject, public QWaylandGlobalInterface
{
public:
    explicit GtkShellGlobal(QObject *parent = 0);

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;
};

class GtkShell : public QObject, public QtWaylandServer::gtk_shell
{
public:
    GtkShell(wl_client *client, uint32_t name, uint32_t version, QObject *parent);
    ~GtkShell();

protected:
    void shell_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void shell_get_gtk_surface(Resource *resource, uint32_t id,
                               wl_resource *surfaceResource)  Q_DECL_OVERRIDE;
};

}

#endif // GTKSHELL_H
