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

#include "clientwindow.h"
#include "xdgsurfaceresizegrabber.h"

namespace GreenIsland {

XdgSurfaceResizeGrabber::XdgSurfaceResizeGrabber(XdgSurface *shellSurface)
    : XdgSurfaceGrabber(shellSurface)
{
}

void XdgSurfaceResizeGrabber::focus()
{
}

void XdgSurfaceResizeGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    QSize delta(m_pt.x() - m_pointer->position().x(),
                m_pt.y() - m_pointer->position().y());

    int32_t newHeight = m_height;
    if (m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_top)
        newHeight = qMax(newHeight + delta.height(), 1);
    else if (m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_bottom)
        newHeight = qMax(newHeight - delta.height(), 1);

    int32_t newWidth = m_width;
    if (m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_left)
        newWidth = qMax(newWidth + delta.width(), 1);
    else if (m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_right)
        newWidth = qMax(newWidth - delta.width(), 1);

    XdgSurface::Changes changes;
    changes.active = m_shellSurface->window() && m_shellSurface->window()->isActive();
    changes.resizing = true;
    changes.size = QSizeF(newWidth, newHeight);
    m_shellSurface->requestConfigure(changes);
}

void XdgSurfaceResizeGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time);

    if (button == Qt::LeftButton && !state) {
        m_pointer->endGrab();
        m_shellSurface->resetResizeGrab();
        delete this;
    }
}

}
