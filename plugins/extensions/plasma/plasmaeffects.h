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

#ifndef PLASMAEFFECTS_H
#define PLASMAEFFECTS_H

#include <QtCore/QLoggingCategory>
#include <GreenIsland/Compositor/QWaylandGlobalInterface>

#include "qwayland-server-plasma-effects.h"

Q_DECLARE_LOGGING_CATEGORY(PLASMA_EFFECTS_PROTOCOL)

namespace GreenIsland {

class PlasmaShell;

class PlasmaEffects : public QWaylandGlobalInterface, public QtWaylandServer::org_kde_plasma_effects
{
public:
    PlasmaEffects(PlasmaShell *shell);

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;

private:
    PlasmaShell *m_shell;

    void effects_slide(Resource *resource,
                       wl_resource *outputResource,
                       wl_resource *surfaceResource,
                       uint32_t from,
                       int32_t x, int32_t y) Q_DECL_OVERRIDE;
    void effects_set_blur_behind_region(Resource *resource,
                                        wl_resource *surfaceResource,
                                        wl_resource *regionResource) Q_DECL_OVERRIDE;
    void effects_set_contrast_region(Resource *resource,
                                     wl_resource *surfaceResource,
                                     wl_resource *regionResource,
                                     uint32_t contrast, uint32_t intensity,
                                     uint32_t saturation) Q_DECL_OVERRIDE;
};

}

#endif // PLASMAEFFECTS_H
