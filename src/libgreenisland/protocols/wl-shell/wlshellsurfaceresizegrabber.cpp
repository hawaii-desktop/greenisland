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

#include <QtGui/QGuiApplication>

#include "compositor.h"
#include "compositor_p.h"
#include "clientwindow.h"
#include "wlshellsurfaceresizegrabber.h"
#include "client/wlcursortheme.h"

namespace GreenIsland {

WlShellSurfaceResizeGrabber::WlShellSurfaceResizeGrabber(WlShellSurface *shellSurface)
    : WlShellSurfaceGrabber(shellSurface)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Change cursor
    if (m_resizeEdges != QtWaylandServer::wl_shell_surface::resize_none) {
        WlCursorTheme::CursorShape shape = WlCursorTheme::BlankCursor;

        switch (m_resizeEdges) {
        case QtWaylandServer::wl_shell_surface::resize_top:
            shape = WlCursorTheme::ResizeNorthCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_top_left:
            shape = WlCursorTheme::ResizeNorthWestCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_top_right:
            shape = WlCursorTheme::ResizeNorthEastCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_bottom:
            shape = WlCursorTheme::ResizeSouthCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_bottom_left:
            shape = WlCursorTheme::ResizeSouthWestCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_bottom_right:
            shape = WlCursorTheme::ResizeSouthEastCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_left:
            shape = WlCursorTheme::ResizeWestCursor;
            break;
        case QtWaylandServer::wl_shell_surface::resize_right:
            shape = WlCursorTheme::ResizeEastCursor;
            break;
        default:
            break;
        }

        if (shape != WlCursorTheme::BlankCursor) {
            Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
            Q_ASSERT(compositor);
            compositor->d_func()->grabCursor(shape);
        }
    }
}

WlShellSurfaceResizeGrabber::~WlShellSurfaceResizeGrabber()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Reset cursor
    Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
    Q_ASSERT(compositor);
    compositor->d_func()->ungrabCursor();
}

void WlShellSurfaceResizeGrabber::focus()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;
}

void WlShellSurfaceResizeGrabber::motion(uint32_t time)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    // Recalculate size
    QSize delta(m_pt.x() - m_pointer->position().x(),
                m_pt.y() - m_pointer->position().y());

    int32_t newHeight = m_height;
    if (m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_top)
        newHeight = qMax(newHeight + delta.height(), 1);
    else if (m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_bottom)
        newHeight = qMax(newHeight - delta.height(), 1);

    int32_t newWidth = m_width;
    if (m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_left)
        newWidth = qMax(newWidth + delta.width(), 1);
    else if (m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_right)
        newWidth = qMax(newWidth - delta.width(), 1);

    // Resize
    m_shellSurface->send_configure(m_resizeEdges, newWidth, newHeight);

    // Adjust position according to resize
    if (m_shellSurface->surface()->transientParent())
        return;
    if (!(m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_top_left))
        return;
    int bottomLeftX = m_pt.x() + m_width;
    int bottomLeftY = m_pt.y() + m_height;
    qreal x = m_shellSurface->window()->position().x();
    qreal y = m_shellSurface->window()->position().y();
    if (m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_top)
        y = bottomLeftY - m_shellSurface->surface()->size().height();
    if (m_resizeEdges & QtWaylandServer::wl_shell_surface::resize_left)
        x = bottomLeftX - m_shellSurface->surface()->size().width();
    m_shellSurface->window()->setPosition(QPointF(x, y));
}

void WlShellSurfaceResizeGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    if (button == Qt::LeftButton && state == 0) {
        //m_pointer->setFocus(0, QPointF());
        m_pointer->endGrab();
        m_shellSurface->resetResizeGrab();
    }
}

}
