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

#ifndef QUICKSURFACE_H
#define QUICKSURFACE_H

#include <QtCompositor/QWaylandQuickSurface>

namespace GreenIsland {

class Compositor;

class QuickSurface : public QWaylandQuickSurface
{
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QPointF globalPosition READ globalPosition WRITE setGlobalPosition NOTIFY globalPositionChanged)
    Q_PROPERTY(QRectF globalGeometry READ globalGeometry NOTIFY globalGeometryChanged)
    Q_ENUMS(State)
public:
    enum State {
        Normal = 0,
        Maximized,
        FullScreen
    };

    explicit QuickSurface(wl_client *client, quint32 id, Compositor *compositor);

    State state() const;
    void setState(const State &state);

    QPointF globalPosition() const;
    void setGlobalPosition(const QPointF &pos);

    QRectF globalGeometry() const;

Q_SIGNALS:
    void stateChanged();
    void globalPositionChanged();
    void globalGeometryChanged();

private:
    State m_state;
    QPointF m_globalPos;
};

}

#endif // QUICKSURFACE_H
