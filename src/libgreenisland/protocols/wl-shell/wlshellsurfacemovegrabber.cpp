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
#include <QtQuick/QQuickItem>

#include "compositor.h"
#include "compositor_p.h"
#include "clientwindow.h"
#include "wlshellsurfacemovegrabber.h"
#include "client/wlcursortheme.h"

namespace GreenIsland {

WlShellSurfaceMoveGrabber::WlShellSurfaceMoveGrabber(WlShellSurface *shellSurface, const QPointF &offset)
    : WlShellSurfaceGrabber(shellSurface)
    , m_offset(offset)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Change cursor
    Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
    Q_ASSERT(compositor);
    compositor->d_func()->grabCursor(WlCursorTheme::ClosedHandCursor);
}

WlShellSurfaceMoveGrabber::~WlShellSurfaceMoveGrabber()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Reset cursor
    Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
    Q_ASSERT(compositor);
    compositor->d_func()->ungrabCursor();
}

void WlShellSurfaceMoveGrabber::focus()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;
}

void WlShellSurfaceMoveGrabber::motion(uint32_t time)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    // Determine pointer coordinates
    QPointF pt(m_pointer->position() - m_offset);

    // Window
    ClientWindow *window = m_shellSurface->window();
    if (!window)
        return;

    // Do not move full screen windows
    if (window->isFullScreen())
        return;

    // Top level windows
    if (window->type() == ClientWindow::TopLevel) {
        // Move the window representation
        if (window->isMaximized()) {
            // Maximized windows if dragged are restored to the original position,
            // but we want to do that with a threshold to avoid unintended grabs
            QPointF threshold(m_offset + QPointF(20, 20));
            if (pt.x() >= threshold.x() || pt.y() >= threshold.y())
                m_shellSurface->restoreAt(pt);
        } else {
            window->setPosition(pt);
        }
    }

    // Move parent, transient will be moved automatically preserving its offset
    // because it's a child QML item
    ClientWindow *parentWindow = window->parentWindow();
    if (parentWindow)
        parentWindow->setPosition(pt - m_shellSurface->surface()->transientOffset());
}

void WlShellSurfaceMoveGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    if (button == Qt::LeftButton && state == 0) {
        //m_pointer->setFocus(0, QPointF());
        m_pointer->endGrab();
        m_shellSurface->resetMoveGrab();
    }
}

}
