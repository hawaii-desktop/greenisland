/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef WLSHELLSURFACERESIZEGRABBER_H
#define WLSHELLSURFACERESIZEGRABBER_H

#include "wlshellsurfacegrabber.h"
#include "client/wlcursortheme.h"

#include "qwayland-server-wayland.h"

namespace GreenIsland {

class WlShellSurfaceResizeGrabber : public WlShellSurfaceGrabber
{
public:
    explicit WlShellSurfaceResizeGrabber(WlShellSurface *shellSurface);
    ~WlShellSurfaceResizeGrabber();

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

private:
    QPointF m_pt;
    QtWaylandServer::wl_shell_surface::resize m_resizeEdges;
    int32_t m_width;
    int32_t m_height;
    WlCursorTheme::CursorShape m_lastShape;

    friend class WlShellSurface;
};

}

#endif // WLSHELLSURFACERESIZEGRABBER_H
