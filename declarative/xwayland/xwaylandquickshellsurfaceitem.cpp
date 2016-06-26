/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL3GPL2$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPLv2 included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
