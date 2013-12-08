/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef SURFACEITEM_H
#define SURFACEITEM_H

#include <QtCompositor/QWaylandSurfaceItem>

class SurfaceItemPrivate;

class SurfaceItem : public QWaylandSurfaceItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SurfaceItem)
    Q_PROPERTY(bool unresponsive READ isUnresponsive NOTIFY unresponsiveChanged)
public:
    SurfaceItem(QQuickItem *parent = 0);
    SurfaceItem(QWaylandSurface *surface, QQuickItem *parent = 0);
    ~SurfaceItem();

    bool isUnresponsive() const;

Q_SIGNALS:
    void unresponsiveChanged(bool value);

protected:
    void mousePressEvent(QMouseEvent *event);
    void touchEvent(QTouchEvent *event);

private:
    SurfaceItemPrivate *const d_ptr;
};

#endif // SURFACEITEM_H
