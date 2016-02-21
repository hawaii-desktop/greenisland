/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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
