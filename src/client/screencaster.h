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

#ifndef GREENISLANDCLIENT_SCREENCASTER_H
#define GREENISLANDCLIENT_SCREENCASTER_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/Shm>

namespace GreenIsland {

namespace Client {

class Output;
class Registry;
class Screencast;
class ScreencastPrivate;
class ScreencasterPrivate;

class GREENISLANDCLIENT_EXPORT Screencaster : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screencaster)
public:
    enum CaptureError {
        ErrorBadOutput = 0
    };
    Q_ENUM(CaptureError)

    Q_INVOKABLE Screencast *capture(Output *output);

    static QByteArray interfaceName();

Q_SIGNALS:
    void captureFailed(CaptureError error);

private:
    Screencaster(Shm *shm, QObject *parent = Q_NULLPTR);

    friend class Registry;
};

class GREENISLANDCLIENT_EXPORT Screencast : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screencast)
public:
    enum Transform {
        TransformNormal = 1,
        TransformYInverted = 2
    };
    Q_ENUM(Transform)

    enum RecordError {
        ErrorBadBuffer = 0
    };
    Q_ENUM(RecordError)

    static QByteArray interfaceName();

Q_SIGNALS:
    void setupDone(const QSize &size, qint32 stride);
    void setupFailed();
    void frameRecorded(Buffer *buffer, quint32 time, Transform transform);
    void failed(RecordError error);
    void canceled();

private:
    Screencast(QObject *parent = Q_NULLPTR);

    friend class Screencaster;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SCREENCASTER_H
