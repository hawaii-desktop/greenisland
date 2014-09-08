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

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceItem>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwloutput_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "output.h"
#include "plasmashell.h"
#include "plasmasurface.h"
#include "windowview.h"

PlasmaShell::PlasmaShell()
{
}

const wl_interface *PlasmaShell::interface() const
{
    return &org_kde_plasma_shell_interface;
}

void PlasmaShell::bind(wl_client *client, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);

    add(client, id);
}

void PlasmaShell::shell_get_surface(Resource *resource, uint32_t id,
                       wl_resource *surfaceResource)
{
    Q_UNUSED(resource);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qWarning() << "Unable to retrieve surface from resource!";
        return;
    }

    new PlasmaSurface(this, surface, resource->client(), id);
}

void PlasmaShell::shell_set_global_position(Resource *resource,
                        wl_resource *surfaceResource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qWarning() << "Unable to retrieve surface from resource!";
        return;
    }

    // We have only one view for shell windows
    for (QWaylandSurfaceView *surfaceView: surface->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;

        QPointF pos = qobject_cast<Output *>(view->output())->mapToOutput(QPointF(x, y));

        QRectF geometry = view->globalGeometry();
        geometry.setTopLeft(pos);

        view->setGlobalGeometry(geometry);
    }
}

void PlasmaShell::shell_desktop_ready(Resource *resource)
{
    Q_UNUSED(resource);
}
