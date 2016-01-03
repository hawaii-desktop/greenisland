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

#ifndef GREENISLANDCLIENT_SCREENSHOOTER_P_H
#define GREENISLANDCLIENT_SCREENSHOOTER_P_H

#include <QtCore/QMutexLocker>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/Screenshooter>
#include <GreenIsland/client/private/qwayland-greenisland-screenshooter.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT ScreenshooterPrivate
        : public QObjectPrivate
        , public QtWayland::greenisland_screenshooter
{
    Q_DECLARE_PUBLIC(Screenshooter)
public:
    ScreenshooterPrivate();
    ~ScreenshooterPrivate();

    QByteArray effectsArray(Screenshooter::Effects effects);
    void removeRequest(Screenshot *screenshot);

    Registry *registry;
    Shm *shm;
    ShmPool *shmPool;
    QMutex requestsMutex;
    QVector<Screenshot *> requests;

    static ScreenshooterPrivate *get(Screenshooter *screenshooter) { return screenshooter->d_func(); }
};

class GREENISLANDCLIENT_EXPORT ScreenshotPrivate
        : public QObjectPrivate
        , public QtWayland::greenisland_screenshot
{
    Q_DECLARE_PUBLIC(Screenshot)
public:
    ScreenshotPrivate(Screenshooter *s);
    ~ScreenshotPrivate();

    Screenshot::CaptureType type;
    Screenshooter::Effects effects;
    Screenshooter *screenshooter;
    Output *output;
    BufferPtr buffer;

    static ScreenshotPrivate *get(Screenshot *screenshot) { return screenshot->d_func(); }

protected:
    void screenshot_setup(int32_t width, int32_t height,
                          int32_t stride, int32_t format) Q_DECL_OVERRIDE;
    void screenshot_done() Q_DECL_OVERRIDE;
    void screenshot_failed(int32_t error) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SCREENSHOOTER_P_H
