/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

#ifndef POINTERITEM_H
#define POINTERITEM_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>

class QWaylandInputDevice;

class PointerItemPrivate;

class PointerItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PointerItem)
    Q_PROPERTY(QWaylandInputDevice *inputDevice READ inputDevice WRITE setInputDevice NOTIFY inputDeviceChanged)
    Q_PROPERTY(int hotspotX READ hotspotX NOTIFY hotspotXChanged)
    Q_PROPERTY(int hotspotY READ hotspotY NOTIFY hotspotYChanged)
public:
    PointerItem(QQuickItem *parent = Q_NULLPTR);

    QWaylandInputDevice *inputDevice() const;
    void setInputDevice(QWaylandInputDevice *device);

    int hotspotX() const;
    int hotspotY() const;

Q_SIGNALS:
    void inputDeviceChanged();
    void hotspotXChanged();
    void hotspotYChanged();

private Q_SLOTS:
    void cursorSurfaceRequest(QWaylandSurface *surface, int hotspotX, int hotspotY);
};

#endif // POINTERITEM_H
