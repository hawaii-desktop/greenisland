/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "xwaylandquickshellintegration.h"
#include "xwaylandquickshellsurfaceitem.h"
#include "xwaylandshellsurface.h"

XWaylandQuickShellSurfaceItem::XWaylandQuickShellSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(parent)
    , m_shellSurface(nullptr)
    , m_shellIntegration(nullptr)
    , m_moveItem(nullptr)
{
}

XWaylandShellSurface *XWaylandQuickShellSurfaceItem::shellSurface() const
{
    return m_shellSurface;
}

void XWaylandQuickShellSurfaceItem::setShellSurface(XWaylandShellSurface *shellSurface)
{
    if (m_shellSurface == shellSurface)
        return;

    m_shellSurface = shellSurface;
    m_shellIntegration = shellSurface->createIntegration(this);
    Q_EMIT shellSurfaceChanged();
}

QQuickItem *XWaylandQuickShellSurfaceItem::moveItem() const
{
    return m_moveItem ? m_moveItem : const_cast<XWaylandQuickShellSurfaceItem *>(this);
}

void XWaylandQuickShellSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    moveItem = moveItem ? moveItem : this;

    if (this->moveItem() == moveItem)
        return;

    m_moveItem = moveItem;
    Q_EMIT moveItemChanged();
}

void XWaylandQuickShellSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_shellIntegration->mouseMoveEvent(event))
        QWaylandQuickItem::mouseMoveEvent(event);
}

void XWaylandQuickShellSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_shellIntegration->mouseReleaseEvent(event))
        QWaylandQuickItem::mouseReleaseEvent(event);
}

#include "moc_xwaylandquickshellsurfaceitem.cpp"
