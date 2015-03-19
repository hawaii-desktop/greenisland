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

#ifndef GREENISLAND_WINDOWVIEW_H
#define GREENISLAND_WINDOWVIEW_H

#include <QtCompositor/QWaylandSurfaceItem>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class GREENISLAND_EXPORT WindowView : public QWaylandSurfaceItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF localPosition READ localPosition NOTIFY localPositionChanged)
    Q_PROPERTY(qreal localX READ localX NOTIFY localPositionChanged)
    Q_PROPERTY(qreal localY READ localY NOTIFY localPositionChanged)
public:
    WindowView(QWaylandQuickSurface *surface, QQuickItem *parent = 0);

    QPointF localPosition() const;
    void setLocalPosition(const QPointF &pt);

    qreal localX() const;
    qreal localY() const;

Q_SIGNALS:
    void localPositionChanged();
    void mousePressed();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    bool m_modifierPressed;
    QPointF m_pos;

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void takeFocus(QWaylandInputDevice *device = 0) Q_DECL_OVERRIDE;

    void startMove();
    void stopMove();
};

}

#endif // GREENISLAND_WINDOWVIEW_H
