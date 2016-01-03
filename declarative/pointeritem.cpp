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

#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/private/qwaylandquickitem_p.h>

#include "pointeritem.h"

class PointerItemPrivate : public QWaylandQuickItemPrivate
{
    Q_DECLARE_PUBLIC(PointerItem)
public:
    PointerItemPrivate()
        : inputDevice(Q_NULLPTR)
        , hotspot(QPoint(0, 0))
    {}

    QWaylandInputDevice *inputDevice;
    QPoint hotspot;
};

PointerItem::PointerItem(QQuickItem *parent)
    : QWaylandQuickItem(*new PointerItemPrivate(), parent)
{
    setInputEventsEnabled(false);
    setVisible(false);
}

QWaylandInputDevice *PointerItem::inputDevice() const
{
    Q_D(const PointerItem);
    return d->inputDevice;
}

void PointerItem::setInputDevice(QWaylandInputDevice *device)
{
    Q_D(PointerItem);

    if (d->inputDevice == device)
        return;

    if (!device && d->inputDevice)
        disconnect(d->inputDevice, &QWaylandInputDevice::cursorSurfaceRequest,
                   this, &PointerItem::cursorSurfaceRequest);
    if (device)
        connect(device, &QWaylandInputDevice::cursorSurfaceRequest,
                this, &PointerItem::cursorSurfaceRequest);

    d->inputDevice = device;
    Q_EMIT inputDeviceChanged();
}

int PointerItem::hotspotX() const
{
    Q_D(const PointerItem);
    return d->hotspot.x();
}

int PointerItem::hotspotY() const
{
    Q_D(const PointerItem);
    return d->hotspot.y();
}

void PointerItem::cursorSurfaceRequest(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    Q_D(PointerItem);

    setSurface(surface);

    if (d->hotspot.x() != hotspotX) {
        d->hotspot.setX(hotspotX);
        Q_EMIT hotspotXChanged();
    }

    if (d->hotspot.y() != hotspotY) {
        d->hotspot.setY(hotspotY);
        Q_EMIT hotspotYChanged();
    }
}

#include "moc_pointeritem.cpp"
