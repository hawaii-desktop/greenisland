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

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandInputDevice>

#include "windowview.h"

namespace GreenIsland {

WindowView::WindowView(QWaylandQuickSurface *surface, QQuickItem *parent)
    : QWaylandSurfaceItem(surface, parent)
    , m_pos(0, 0)
{
    qRegisterMetaType<WindowView *>("WindowView*");
}

QPointF WindowView::localPosition() const
{
    return m_pos;
}

void WindowView::setLocalPosition(const QPointF &pt)
{
    if (m_pos == pt)
        return;

    m_pos = pt;
    Q_EMIT localPositionChanged();
}

qreal WindowView::localX() const
{
    return localPosition().x();
}

qreal WindowView::localY() const
{
    return localPosition().y();
}

void WindowView::mousePressEvent(QMouseEvent *event)
{
    // Emit a mouse pressed signal, ClientWindow will connect
    // and activate the window
    Q_EMIT mousePressed();

    QWaylandSurfaceItem::mousePressEvent(event);
}

void WindowView::takeFocus(QWaylandInputDevice *device)
{
    QWaylandSurfaceItem::takeFocus(device);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    forceActiveFocus();
#endif
}

}

#include "moc_windowview.cpp"
