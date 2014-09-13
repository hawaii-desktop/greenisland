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
    Q_PROPERTY(QPointF globalPosition READ globalPosition WRITE setGlobalPosition NOTIFY globalPositionChanged)
    Q_PROPERTY(QRectF globalGeometry READ globalGeometry NOTIFY globalGeometryChanged)
public:
    explicit QuickSurface(wl_client *client, quint32 id, Compositor *compositor);

    QPointF globalPosition() const;
    void setGlobalPosition(const QPointF &pos);

    QRectF globalGeometry() const;

Q_SIGNALS:
    void globalPositionChanged();
    void globalGeometryChanged();

private:
    QPointF m_globalPos;
};

}

#endif // QUICKSURFACE_H
