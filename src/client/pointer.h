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
