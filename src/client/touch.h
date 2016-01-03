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

#ifndef GREENISLANDCLIENT_TOUCH_H
#define GREENISLANDCLIENT_TOUCH_H

#include <QtCore/QObject>

#include <GreenIsland/client/greenislandclient_export.h>

namespace GreenIsland {

namespace Client {

class Seat;
class Surface;
class TouchPrivate;

class TouchPoint;
class TouchPointPrivate;

class GREENISLANDCLIENT_EXPORT Touch : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Touch)
public:
    Touch(Seat *seat);

    static QByteArray interfaceName();

Q_SIGNALS:
    void sequenceStarted(TouchPoint *firstPoint);
    void sequenceCanceled();
    void sequenceFinished();
    void frameFinished();
    void pointAdded(TouchPoint *point);
    void pointRemoved(TouchPoint *point);
    void pointMoved(TouchPoint *point);
};

class GREENISLANDCLIENT_EXPORT TouchPoint
{
    Q_DECLARE_PRIVATE(TouchPoint)
public:
    TouchPoint();
    ~TouchPoint();

    qint32 id() const;
    quint32 upSerial() const;
    quint32 downSerial() const;
    quint32 timestamp() const;
    QVector<quint32> timestamps() const;
    QPointF position() const;
    QVector<QPointF> positions() const;
    QPointer<Surface> surface() const;
    bool isDown() const;

private:
    TouchPointPrivate *const d_ptr;

    friend class TouchPrivate;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_TOUCH_H
