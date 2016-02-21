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

#ifndef GREENISLANDCLIENT_POINTER_H
#define GREENISLANDCLIENT_POINTER_H

#include <QtCore/QObject>
#include <QtCore/QPoint>

#include <GreenIsland/client/greenislandclient_export.h>

struct wl_cursor_image;

namespace GreenIsland {

namespace Client {

class PointerPrivate;
class Seat;
class Surface;

class GREENISLANDCLIENT_EXPORT Pointer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Pointer)
    Q_PROPERTY(Surface *focusSurface READ focusSurface NOTIFY focusSurfaceChanged)
    Q_PROPERTY(Surface *cursorSurface READ cursorSurface NOTIFY cursorSurfaceChanged)
public:
    Pointer(Seat *seat);

    Surface *focusSurface() const;

    Surface *cursorSurface() const;
    void setCursor(Surface *surface, const QPoint &hotSpot = QPoint(0, 0));

    static QByteArray interfaceName();

Q_SIGNALS:
    void focusSurfaceChanged();
    void cursorSurfaceChanged();
    void enter(quint32 serial, const QPointF &relativeToSurface);
    void leave(quint32 serial);
    void motion(quint32 time, const QPointF &relativeToSurface);
    void buttonPressed(quint32 serial, quint32 time, const Qt::MouseButton &button);
    void buttonReleased(quint32 serial, quint32 time, const Qt::MouseButton &button);
    void axisChanged(quint32 time, const Qt::Orientation &orientation, qreal value);
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_POINTER_H
