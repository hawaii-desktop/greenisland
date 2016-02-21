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

#ifndef GREENISLAND_SCREENSHOOTER_H
#define GREENISLAND_SCREENSHOOTER_H

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

#include <GreenIsland/server/greenislandserver_export.h>

class QWaylandOutput;
class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class Screenshot;
class ScreenshotPrivate;
class ScreenshooterPrivate;

class GREENISLANDSERVER_EXPORT Screenshooter : public QWaylandExtensionTemplate<Screenshooter>
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

    Screenshooter();
    Screenshooter(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void captureRequested(Screenshot *screenshot);
};

class GREENISLANDSERVER_EXPORT Screenshot : public QWaylandExtensionTemplate<Screenshot>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screenshot)
    Q_PROPERTY(CaptureType captureType READ captureType CONSTANT)
    Q_PROPERTY(Screenshooter::Effects effects READ effects CONSTANT)
public:
    enum CaptureType {
        CaptureOutput = 1,
        CaptureActiveWindow,
        CaptureWindow,
        CaptureArea
    };
    Q_ENUM(CaptureType)

    enum Error {
        ErrorBadBuffer = 0
    };
    Q_ENUM(Error)

    CaptureType captureType() const;
    Screenshooter::Effects effects() const;

    Q_INVOKABLE void selectSurface(QWaylandSurface *surface);
    Q_INVOKABLE void selectArea(QQuickItem *area);
    Q_INVOKABLE void setup();

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void done();
    void failed(Error error);

private:
    explicit Screenshot(CaptureType type, Screenshooter::Effects effects);

    friend class ScreenshooterPrivate;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENSHOOTER_H
