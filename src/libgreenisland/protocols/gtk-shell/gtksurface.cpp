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

#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "gtksurface.h"

namespace GreenIsland {

GtkSurface::GtkSurface(GtkShell *shell, QWaylandSurface *surface,
                       wl_client *client, uint32_t id, uint32_t version)
    : QObject(shell)
    , QWaylandSurfaceInterface(surface)
    , QtWaylandServer::gtk_surface(client, id, version)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif
}

GtkSurface::~GtkSurface()
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
}

bool GtkSurface::runOperation(QWaylandSurfaceOp *op)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
    qCDebug(GTKSHELL_PROTOCOL) << "Run operation" << op->type();
#endif

    return false;
}

void GtkSurface::surface_destroy_resource(Resource *resource)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    Q_UNUSED(resource)
    delete this;
}

void GtkSurface::surface_set_dbus_properties(Resource *resource,
                                             const QString &application_id,
                                             const QString &app_menu_path,
                                             const QString &menubar_path,
                                             const QString &window_object_path,
                                             const QString &application_object_path,
                                             const QString &unique_bus_name)
{
#ifdef ENABLE_GTK_SHELL_TRACE
    qCDebug(GTKSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    Q_UNUSED(resource)

    qCDebug(GTKSHELL_PROTOCOL)
            << "Application ID:" << application_id
            << "App menu path:" << app_menu_path
            << "Menubar path:" << menubar_path
            << "Window path:" << window_object_path
            << "Application object path:" << application_object_path
            << "Unique bus name:" << unique_bus_name;
    m_applicationId = application_id;

    // Find the ClientWindow for this surface and trigger the heuristic to
    // determine the app_id (which in turn will iterate through the surface
    // interfaces and get the app_id from GtkSurface)
    // TODO: Add the following signals to QtCompositor so that ugly code
    // can be done from ClientWindow:
    // - QWaylandSurface::interfaceAdded(QWaylandSurfaceInterface *)
    // - QWaylandSurface::interfaceRemoved(QWaylandSurfaceInterface *)
    if (surface()) {
        Compositor *compositor = static_cast<Compositor *>(surface()->compositor());
        Q_FOREACH (ClientWindow *clientWindow, compositor->d_func()->clientWindowsList) {
            if (clientWindow->surface() == surface()) {
                clientWindow->surfaceAppIdChanged();
                break;
            }
        }
    }
}

}
