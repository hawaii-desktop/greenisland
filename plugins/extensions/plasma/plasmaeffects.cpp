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

#include <GreenIsland/Compositor/QWaylandOutput>
#include <GreenIsland/Compositor/QWaylandSurface>
#include <waylandcompositor/wayland_wrapper/qwlregion_p.h>

#include "plasmaeffects.h"
#include "plasmashell.h"
#include "plasmasurface.h"

Q_LOGGING_CATEGORY(PLASMA_EFFECTS_PROTOCOL, "greenisland.plugins.plasma.effects")

namespace GreenIsland {

PlasmaEffects::PlasmaEffects(PlasmaShell *shell)
    : m_shell(shell)
{
}

const wl_interface *PlasmaEffects::interface() const
{
    return &org_kde_plasma_effects_interface;
}

void PlasmaEffects::bind(wl_client *client, uint32_t version, uint32_t id)
{
    add(client, id, version);
}

void PlasmaEffects::effects_slide(Resource *resource,
                                  wl_resource *outputResource,
                                  wl_resource *surfaceResource,
                                  uint32_t from,
                                  int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    if (!output) {
        qCWarning(PLASMA_EFFECTS_PROTOCOL) << "Couldn't get output from resource";
        return;
    }

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qCWarning(PLASMA_EFFECTS_PROTOCOL) << "Couldn't get surface from resource";
        return;
    }

    QPointF ptFrom;
    QPointF ptTo(x, y);

    switch (from) {
    case location_none:
        ptFrom.setX(output->geometry().width() / 2);
        ptFrom.setY(output->geometry().height() / 2);
        break;
    case location_left:
        ptFrom = QPointF(-surface->size().width(), 0);
        break;
    case location_top:
        ptFrom = QPointF(0, -surface->size().height());
        break;
    case location_right:
        ptFrom = QPointF(output->geometry().width() +
                         surface->size().width(), 0);
        break;
    case location_bottom:
        ptFrom = QPointF(0, output->geometry().height() +
                         surface->size().height());
        break;
    }

    for (const PlasmaSurface *plasmaSurface: m_shell->surfaces()) {
        if (plasmaSurface->surface() == surface)
            Q_EMIT plasmaSurface->window()->moveRequested(ptFrom, ptTo);
    }
}

void PlasmaEffects::effects_set_blur_behind_region(Resource *resource,
                                                   wl_resource *surfaceResource,
                                                   wl_resource *regionResource)
{
    Q_UNUSED(resource);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qCWarning(PLASMA_EFFECTS_PROTOCOL) << "Couldn't get surface from resource";
        return;
    }

    QtWayland::Region *r = QtWayland::Region::fromResource(regionResource);
    r->region();

    // TODO: Set blur behind region on views

    /*
     * TODO: Maybe add a property map to ShellWindow called "effects"
     * with the "blur_behind" entry that contains the "region" property.
     */
}

void PlasmaEffects::effects_set_contrast_region(Resource *resource,
                                                wl_resource *surfaceResource,
                                                wl_resource *regionResource,
                                                uint32_t contrast,
                                                uint32_t intensity,
                                                uint32_t saturation)
{
    Q_UNUSED(resource);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qCWarning(PLASMA_EFFECTS_PROTOCOL) << "Couldn't get surface from resource";
        return;
    }

    QtWayland::Region *r = QtWayland::Region::fromResource(regionResource);
    r->region();

    // TODO: Set contrast on views
    Q_UNUSED(contrast);
    Q_UNUSED(intensity);
    Q_UNUSED(saturation);

    /*
     * TODO: Maybe add a property map to ShellWindow called "effects"
     * with the "contrast" entry that contains the "region" property
     * and "contrast", "intensity", "saturation" properties.
     */
}

}
