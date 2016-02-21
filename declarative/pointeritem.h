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
