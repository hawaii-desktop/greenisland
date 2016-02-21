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
