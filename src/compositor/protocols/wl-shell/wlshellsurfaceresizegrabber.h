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

#ifndef WLSHELLSURFACERESIZEGRABBER_H
#define WLSHELLSURFACERESIZEGRABBER_H

#include <QtCompositor/private/qwayland-server-wayland.h>

#include "wlshellsurfacegrabber.h"

class WlShellSurfaceResizeGrabber : public WlShellSurfaceGrabber
{
public:
    explicit WlShellSurfaceResizeGrabber(WlShellSurface *shellSurface);

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

private:
    QPointF m_pt;
    QtWaylandServer::wl_shell_surface::resize m_resizeEdges;
    int32_t m_width;
    int32_t m_height;

    friend class WlShellSurface;
};

#endif // WLSHELLSURFACERESIZEGRABBER_H
