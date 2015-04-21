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
#include <QtQuick/QQuickItem>

#include "compositor.h"
#include "compositor_p.h"
#include "clientwindow.h"
#include "xdgsurfacemovegrabber.h"
#include "client/wlcursortheme.h"

namespace GreenIsland {

XdgSurfaceMoveGrabber::XdgSurfaceMoveGrabber(XdgSurface *shellSurface, const QPointF &offset)
    : XdgSurfaceGrabber(shellSurface)
    , m_offset(offset)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Change cursor
    Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
    Q_ASSERT(compositor);
    if (compositor->d_func()->clientData.cursorTheme) {
        compositor->d_func()->clientData.cursorTheme->changeCursor(WlCursorTheme::ClosedHandCursor);
        compositor->d_func()->grabCursor = true;
    }
}

XdgSurfaceMoveGrabber::~XdgSurfaceMoveGrabber()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Reset cursor
    Compositor *compositor = static_cast<Compositor *>(m_shellSurface->surface()->compositor());
    Q_ASSERT(compositor);
    if (compositor->d_func()->clientData.cursorTheme) {
        compositor->d_func()->clientData.cursorTheme->changeCursor(WlCursorTheme::BlankCursor);
        compositor->d_func()->grabCursor = false;
    }
}

void XdgSurfaceMoveGrabber::focus()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;
}

void XdgSurfaceMoveGrabber::motion(uint32_t time)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    // Determine pointer coordinates
    QPointF pt(m_pointer->position() - m_offset);

    // Window
    ClientWindow *window = m_shellSurface->window();
    if (!window)
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

void XdgSurfaceMoveGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        //m_pointer->setFocus(0, QPointF());
        m_pointer->endGrab();
        m_shellSurface->resetMoveGrab();
        delete this;
    }
}

}
