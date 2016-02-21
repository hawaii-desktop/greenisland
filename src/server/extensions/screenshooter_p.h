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

#ifndef GREENISLAND_SCREENSHOOTER_P_H
#define GREENISLAND_SCREENSHOOTER_P_H

#include <QtQuick/QQuickItem>

#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>
#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>
#include <GreenIsland/QtWaylandCompositor/private/qwaylandextension_p.h>

#include <GreenIsland/Server/Screenshooter>
#include <GreenIsland/server/private/qwayland-server-greenisland-screenshooter.h>

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

class GREENISLANDSERVER_EXPORT ScreenshooterPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_screenshooter
{
    Q_DECLARE_PUBLIC(Screenshooter)
public:
    ScreenshooterPrivate();

    static ScreenshooterPrivate *get(Screenshooter *screenshooter) { return screenshooter->d_func(); }

protected:
    void screenshooter_capture_output(Resource *resource,
                                      uint32_t id,
                                      struct ::wl_resource *outputResource,
                                      wl_array *effects) Q_DECL_OVERRIDE;
    void screenshooter_capture_active(Resource *resource,
                                      uint32_t id,
                                      wl_array *effects) Q_DECL_OVERRIDE;
    void screenshooter_capture_surface(Resource *resource,
                                       uint32_t id,
                                       wl_array *effects) Q_DECL_OVERRIDE;
    void screenshooter_capture_area(Resource *resource,
                                    uint32_t id,
                                    wl_array *effects) Q_DECL_OVERRIDE;
};

class GREENISLANDSERVER_EXPORT ScreenshotPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_screenshot
{
    Q_DECLARE_PUBLIC(Screenshot)
public:
    ScreenshotPrivate();

    QImage grabItem(QQuickItem *item);

    Screenshot::CaptureType captureType;
    Screenshooter::Effects effects;
    QWaylandOutput *output;
    QWaylandSurface *selectedSurface;
    QQuickItem *selectedArea;

    static ScreenshotPrivate *get(Screenshot *screenshot) { return screenshot->d_func(); }

protected:
    void screenshot_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void screenshot_record(Resource *resource,
                           struct ::wl_resource *bufferResource) Q_DECL_OVERRIDE;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENSHOOTER_P_H
