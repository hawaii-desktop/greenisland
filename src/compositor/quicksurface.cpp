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

#include "compositor.h"
#include "quicksurface.h"

namespace GreenIsland {

QuickSurface::QuickSurface(wl_client *client, quint32 id, Compositor *compositor)
    : QWaylandQuickSurface(client, id, compositor)
    , m_state(Normal)
{
}

QuickSurface::State QuickSurface::state() const
{
    return m_state;
}

void QuickSurface::setState(const QuickSurface::State &state)
{
    if (m_state == state)
        return;

    m_state = state;
    Q_EMIT stateChanged();
}

QPointF QuickSurface::globalPosition() const
{
    return m_globalPos;
}

void QuickSurface::setGlobalPosition(const QPointF &pos)
{
    if (m_globalPos == pos)
        return;

    m_globalPos = pos;
    Q_EMIT globalPositionChanged();
    Q_EMIT globalGeometryChanged();
}

QRectF QuickSurface::globalGeometry() const
{
    return QRectF(m_globalPos, QSizeF(size()));
}

}

#include "moc_quicksurface.cpp"
