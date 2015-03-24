/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCompositor/QWaylandSurface>

#include "gtkshell.h"
#include "gtksurface.h"

Q_LOGGING_CATEGORY(GTKSHELL_PROTOCOL, "greenisland.protocols.gtkshell")

namespace GreenIsland {

GtkShellGlobal::GtkShellGlobal(QObject *parent)
    : QObject(parent)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif
}

const wl_interface *GtkShellGlobal::interface() const
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    return &gtk_shell_interface;
}

void GtkShellGlobal::bind(wl_client *client, uint32_t version, uint32_t id)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    new GtkShell(client, id, version, this);
}

GtkShell::GtkShell(wl_client *client, uint32_t name, uint32_t version, QObject *parent)
    : QObject(parent)
    , QtWaylandServer::gtk_shell(client, name, version)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    // We have no capabilities
    send_capabilities(0);
}

GtkShell::~GtkShell()
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
}

void GtkShell::shell_destroy_resource(Resource *resource)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    Q_UNUSED(resource)
    delete this;
}

void GtkShell::shell_get_gtk_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    new GtkSurface(this, surface, resource->client(), id, resource->version());
}

}
