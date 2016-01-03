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

#ifndef GREENISLAND_SCREENCASTER_P_H
#define GREENISLAND_SCREENCASTER_P_H

#include <QtQuick/QQuickWindow>

#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>
#include <GreenIsland/QtWaylandCompositor/private/qwaylandextension_p.h>

#include <GreenIsland/Server/Screencaster>
#include <GreenIsland/server/private/qwayland-server-greenisland-screencaster.h>

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

namespace Server {

class GREENISLANDSERVER_EXPORT ScreencasterPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_screencaster
{
    Q_DECLARE_PUBLIC(Screencaster)
public:
    ScreencasterPrivate();

    QMutex requestsMutex;
    QMultiHash<QQuickWindow *, Screencast *> requests;

    void addRequest(QQuickWindow *window, Screencast *screencast);
    void removeRequest(QQuickWindow *window, Screencast *screencast);

    static ScreencasterPrivate *get(Screencaster *screencaster) { return screencaster->d_func(); }

protected:
    void screencaster_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
    void screencaster_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void screencaster_capture(Resource *resource,
                              uint32_t id,
                              struct ::wl_resource *outputResource) Q_DECL_OVERRIDE;
};

class GREENISLANDSERVER_EXPORT ScreencastPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_screencast
{
    Q_DECLARE_PUBLIC(Screencast)
public:
    ScreencastPrivate();

    bool valid;
    Screencaster *screencaster;
    QWaylandOutput *output;
    QQuickWindow *window;
    wl_resource *bufferResource;
    wl_shm_buffer *buffer;

    void frameRecording(Screencast *screencast, uint time,
                        const QImage &image);

    static ScreencastPrivate *get(Screencast *screencast) { return screencast->d_func(); }

protected:
    void screencast_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void screencast_record(Resource *resource,
                           struct ::wl_resource *bufferResource) Q_DECL_OVERRIDE;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENCASTER_P_H
