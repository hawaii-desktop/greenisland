/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>

#include "gtkshell.h"
#include "gtkshell_p.h"
#include "serverlogging_p.h"

namespace GreenIsland {

namespace Server {

/*
 * GtkShellPrivate
 */

GtkShellPrivate::GtkShellPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::gtk_shell()
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;
}

void GtkShellPrivate::shell_bind_resource(Resource *resource)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    send_capabilities(resource->handle, 0);
}

void GtkShellPrivate::shell_destroy_resource(Resource *resource)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(GtkShell);
    delete q;
}

void GtkShellPrivate::shell_get_gtk_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_Q(GtkShell);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    Q_ASSERT(compositor);

    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    Q_ASSERT(client);

    Q_EMIT q->createSurface(surface, client, id);
}

/*
 * GtkShell
 */

GtkShell::GtkShell()
    : QWaylandExtensionTemplate<GtkShell>(*new GtkShellPrivate())
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;
}

GtkShell::GtkShell(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<GtkShell>(compositor, *new GtkShellPrivate())
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;
}

void GtkShell::initialize()
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_D(GtkShell);

    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing GtkShell";
        return;
    }
    d->init(compositor->display(), QtWaylandServer::gtk_shell::interfaceVersion());
}

const struct wl_interface *GtkShell::interface()
{
    return GtkShellPrivate::interface();
}

QByteArray GtkShell::interfaceName()
{
    return GtkShellPrivate::interfaceName();
}

/*
 * GtkSurfacePrivate
 */

GtkSurfacePrivate::GtkSurfacePrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::gtk_surface()
    , m_shell(Q_NULLPTR)
    , m_surface(Q_NULLPTR)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;
}

GtkSurfacePrivate::~GtkSurfacePrivate()
{
}

void GtkSurfacePrivate::surface_destroy_resource(Resource *resource)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(GtkSurface);
    delete q;
}

void GtkSurfacePrivate::surface_set_dbus_properties(Resource *resource,
                                                    const QString &application_id,
                                                    const QString &app_menu_path,
                                                    const QString &menubar_path,
                                                    const QString &window_object_path,
                                                    const QString &application_object_path,
                                                    const QString &unique_bus_name)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(GtkSurface);

    qCDebug(gLcGtkShell)
            << "Application ID:" << application_id
            << "App menu path:" << app_menu_path
            << "Menubar path:" << menubar_path
            << "Window path:" << window_object_path
            << "Application object path:" << application_object_path
            << "Unique bus name:" << unique_bus_name;

    m_appId = application_id;
    m_appMenuPath = app_menu_path;
    m_windowObjectPath = window_object_path;
    m_appObjectPath = application_object_path;
    m_uniqueBusName = unique_bus_name;

    Q_EMIT q->appIdChanged(m_appId);
    Q_EMIT q->appMenuPathChanged(m_appMenuPath);
    Q_EMIT q->menuBarPathChanged(m_menuBarPath);
    Q_EMIT q->windowObjectPathChanged(m_windowObjectPath);
    Q_EMIT q->appObjectPathChanged(m_appObjectPath);
    Q_EMIT q->uniqueBusNameChanged(m_uniqueBusName);
}

void GtkSurfacePrivate::surface_set_modal(Resource *resource)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(GtkSurface);
    Q_EMIT q->setModal();
}

void GtkSurfacePrivate::surface_unset_modal(Resource *resource)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(GtkSurface);
    Q_EMIT q->unsetModal();
}

/*
 * GtkSurface
 */

GtkSurface::GtkSurface()
    : QWaylandExtensionTemplate<GtkSurface>(*new GtkSurfacePrivate())
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;
}

GtkSurface::GtkSurface(GtkShell *shell, QWaylandSurface *surface,
                       QWaylandClient *client, uint id)
    : QWaylandExtensionTemplate<GtkSurface>(*new GtkSurfacePrivate())
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    initialize(shell, surface, client, id);
}

void GtkSurface::initialize(GtkShell *shell, QWaylandSurface *surface,
                            QWaylandClient *client, uint id)
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    Q_D(GtkSurface);
    d->m_shell = shell;
    d->m_surface = surface;
    d->init(client->client(), id, 1);
    setExtensionContainer(surface);
    Q_EMIT surfaceChanged();
    QWaylandExtension::initialize();
}

void GtkSurface::initialize()
{
    qCDebug(gLcGtkShellTrace) << Q_FUNC_INFO;

    QWaylandExtensionTemplate::initialize();
}

QWaylandSurface *GtkSurface::surface() const
{
    Q_D(const GtkSurface);
    return d->m_surface;
}

QString GtkSurface::appId() const
{
    Q_D(const GtkSurface);
    return d->m_appId;
}

QString GtkSurface::appMenuPath() const
{
    Q_D(const GtkSurface);
    return d->m_appMenuPath;
}

QString GtkSurface::menuBarPath() const
{
    Q_D(const GtkSurface);
    return d->m_menuBarPath;
}

QString GtkSurface::windowObjectPath() const
{
    Q_D(const GtkSurface);
    return d->m_windowObjectPath;
}

QString GtkSurface::appObjectPath() const
{
    Q_D(const GtkSurface);
    return d->m_appObjectPath;
}

QString GtkSurface::uniqueBusName() const
{
    Q_D(const GtkSurface);
    return d->m_uniqueBusName;
}

const struct wl_interface *GtkSurface::interface()
{
    return GtkSurfacePrivate::interface();
}

QByteArray GtkSurface::interfaceName()
{
    return GtkSurfacePrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_gtkshell.cpp"
