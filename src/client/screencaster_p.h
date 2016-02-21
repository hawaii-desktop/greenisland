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

#ifndef GREENISLANDCLIENT_SCREENCASTER_P_H
#define GREENISLANDCLIENT_SCREENCASTER_P_H

#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/Screencaster>
#include <GreenIsland/client/private/qwayland-greenisland-screencaster.h>

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

class ScreencastBuffer;
class ScreencastHandler;

class GREENISLANDCLIENT_EXPORT ScreencasterPrivate
        : public QObjectPrivate
        , public QtWayland::greenisland_screencaster
{
    Q_DECLARE_PUBLIC(Screencaster)
public:
    ScreencasterPrivate();
    ~ScreencasterPrivate();

    Registry *registry;
    Shm *shm;
    QVector<Screencast *> requests;

    static ScreencasterPrivate *get(Screencaster *screencaster) { return screencaster->d_func(); }
};

class GREENISLANDCLIENT_EXPORT ScreencastPrivate
        : public QObjectPrivate
        , public QtWayland::greenisland_screencast
{
    Q_DECLARE_PUBLIC(Screencast)
public:
    ScreencastPrivate(Screencaster *s);
    ~ScreencastPrivate();

    ShmPool *shmPool;
    Screencaster *screencaster;
    QMutex recordMutex;
    QVector<ScreencastBuffer *> buffers;
    bool starving;

    QThread *thread;
    ScreencastHandler *handler;

    void recordFrame();

    static ScreencastPrivate *get(Screencast *screencast) { return screencast->d_func(); }

protected:
    void screencast_setup(int32_t width, int32_t height,
                          int32_t stride, int32_t format) Q_DECL_OVERRIDE;
    void screencast_frame(struct ::wl_buffer *buffer,
                          uint32_t time, int32_t transform) Q_DECL_OVERRIDE;
    void screencast_failed(int32_t error,
                           struct ::wl_buffer *buffer) Q_DECL_OVERRIDE;
    void screencast_cancelled(struct ::wl_buffer *buffer) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SCREENCASTER_P_H
