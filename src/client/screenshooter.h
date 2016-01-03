/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#ifndef GREENISLANDCLIENT_SCREENSHOOTER_H
#define GREENISLANDCLIENT_SCREENSHOOTER_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/Shm>

namespace GreenIsland {

namespace Client {

class Output;
class Registry;
class ScreenshooterPrivate;
class Screenshot;
class ScreenshotPrivate;

class GREENISLANDCLIENT_EXPORT Screenshooter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screenshooter)
public:
    enum Effect {
        EffectPointer = 1,
        EffectBorder
    };
    Q_ENUM(Effect)
    Q_DECLARE_FLAGS(Effects, Effect)

    Screenshot *captureOutput(Output *output, Effects effects);
    Screenshot *captureActiveWindow(Effects effects);
    Screenshot *captureWindow(Effects effects);
    Screenshot *captureArea(Effects effects);

    static QByteArray interfaceName();

private:
    Screenshooter(Shm *shm, QObject *parent = Q_NULLPTR);

    friend class Registry;
};

class GREENISLANDCLIENT_EXPORT Screenshot : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screenshot)
    Q_PROPERTY(CaptureType captureType READ captureType CONSTANT)
    Q_PROPERTY(Screenshooter::Effects effects READ effects CONSTANT)
    Q_PROPERTY(Output *output READ output CONSTANT)
public:
    enum CaptureType {
        CaptureOutput = 1,
        CaptureActiveWindow,
        CaptureWindow,
        CaptureArea
    };
    Q_ENUM(CaptureType)

    enum Error {
        ErrorBadBuffer
    };
    Q_ENUM(Error)

    CaptureType captureType() const;
    Screenshooter::Effects effects() const;
    Output *output() const;

    static QByteArray interfaceName();

Q_SIGNALS:
    void validChanged();
    void setupDone(const QSize &size, qint32 stride);
    void setupFailed();
    void done(Buffer *buffer);
    void failed(Error error);

private:
    Screenshot(CaptureType type, Screenshooter::Effects effects,
               QObject *parent = Q_NULLPTR);

    friend class Screenshooter;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SCREENSHOOTER_H
