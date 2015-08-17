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

#include <QtGui/QGuiApplication>

#include "compositor.h"
#include "compositor_p.h"
#include "clientwindow.h"
#include "xdgsurfaceresizegrabber.h"
#include "client/wlcursortheme.h"

namespace GreenIsland {

XdgSurfaceResizeGrabber::XdgSurfaceResizeGrabber(XdgSurface *shellSurface)
    : XdgSurfaceGrabber(shellSurface)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Change cursor
    if (m_resizeEdges != QtWaylandServer::xdg_surface::resize_edge_none) {
        Client::WlCursorTheme::CursorShape shape = Client::WlCursorTheme::BlankCursor;

        switch (m_resizeEdges) {
        case QtWaylandServer::xdg_surface::resize_edge_top:
            shape = Client::WlCursorTheme::ResizeNorthCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_top_left:
            shape = Client::WlCursorTheme::ResizeNorthWestCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_top_right:
            shape = Client::WlCursorTheme::ResizeNorthEastCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_bottom:
            shape = Client::WlCursorTheme::ResizeSouthCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_bottom_left:
            shape = Client::WlCursorTheme::ResizeSouthWestCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_bottom_right:
            shape = Client::WlCursorTheme::ResizeSouthEastCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_left:
            shape = Client::WlCursorTheme::ResizeWestCursor;
            break;
        case QtWaylandServer::xdg_surface::resize_edge_right:
            shape = Client::WlCursorTheme::ResizeEastCursor;
            break;
        default:
            break;
        }

        if (shape != Client::WlCursorTheme::BlankCursor) {
            Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
            Q_ASSERT(compositor);
            compositor->d_func()->grabCursor(shape);
        }
    }
}

XdgSurfaceResizeGrabber::~XdgSurfaceResizeGrabber()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Reset cursor
    Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
    Q_ASSERT(compositor);
    compositor->d_func()->ungrabCursor();
}

void XdgSurfaceResizeGrabber::focus()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;
}

void XdgSurfaceResizeGrabber::motion(uint32_t time)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    // Recalculate size
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

    // Resize
    XdgSurface::Changes changes;
    changes.active = m_shellSurface->window() && m_shellSurface->window()->isActive();
    changes.resizing = true;
    changes.size = QSizeF(newWidth, newHeight);
    m_shellSurface->requestConfigure(changes);

    // Adjust position according to resize
    if (m_shellSurface->surface()->transientParent())
        return;
    if (!(m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_top_left))
        return;
    int bottomLeftX = m_pt.x() + m_width;
    int bottomLeftY = m_pt.y() + m_height;
    qreal x = m_shellSurface->window()->position().x();
    qreal y = m_shellSurface->window()->position().y();
    if (m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_top)
        y = bottomLeftY - m_shellSurface->surface()->size().height();
    if (m_resizeEdges & QtWaylandServer::xdg_surface::resize_edge_left)
        x = bottomLeftX - m_shellSurface->surface()->size().width();
    m_shellSurface->window()->setPosition(QPointF(x, y));
}

void XdgSurfaceResizeGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        //m_pointer->setFocus(0, QPointF());
        m_pointer->endGrab();
        m_shellSurface->resetResizeGrab();
        delete this;
    }
}

}
