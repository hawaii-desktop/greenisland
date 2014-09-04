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

#include "wlshellsurfacemovegrabber.h"

WlShellSurfaceMoveGrabber::WlShellSurfaceMoveGrabber(WlShellSurface *shellSurface, const QPointF &offset)
    : WlShellSurfaceGrabber(shellSurface)
    , m_offset(offset)
{
}

void WlShellSurfaceMoveGrabber::focus()
{
}

void WlShellSurfaceMoveGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    QCursor cursor(Qt::ClosedHandCursor);
    QGuiApplication::setOverrideCursor(cursor);

    // Move the window representation
    QPointF pt(m_pointer->position() - m_offset);
    m_shellSurface->setPosition(pt);

    // Set transient window offset
    if (m_shellSurface->transientParent())
        m_shellSurface->setOffset(pt - m_shellSurface->transientParent()->position());
}

void WlShellSurfaceMoveGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time);

    if (button == Qt::LeftButton && !state) {
        m_pointer->setFocus(0, QPointF());
        m_pointer->endGrab();
        m_shellSurface->m_moveGrabber = Q_NULLPTR;
        delete this;

        QCursor cursor(Qt::ArrowCursor);
        QGuiApplication::setOverrideCursor(cursor);
    } else {
        QCursor cursor(Qt::OpenHandCursor);
        QGuiApplication::setOverrideCursor(cursor);
    }
}
