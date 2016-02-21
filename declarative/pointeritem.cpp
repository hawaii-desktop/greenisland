/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
