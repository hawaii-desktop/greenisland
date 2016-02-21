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

#ifndef GREENISLAND_SCREENCASTER_H
#define GREENISLAND_SCREENCASTER_H

#include <QtCore/QObject>
#include <QtCore/QMultiHash>
#include <QtCore/QMutex>

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

#include <GreenIsland/server/greenislandserver_export.h>

class QQuickWindow;

namespace GreenIsland {

namespace Server {

class Screencast;
class ScreencastPrivate;
class ScreencasterPrivate;

class GREENISLANDSERVER_EXPORT Screencaster : public QWaylandExtensionTemplate<Screencaster>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screencaster)
public:
    Screencaster();
    Screencaster(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    void recordFrame(QQuickWindow *window);

    static const struct wl_interface* interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void captureRequested(Screencast *screencast);
};

class GREENISLANDSERVER_EXPORT Screencast : public QWaylandExtensionTemplate<Screencast>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screencast)
public:
    ~Screencast();

    static const struct wl_interface* interface();
    static QByteArray interfaceName();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    explicit Screencast();

    friend class ScreencasterPrivate;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENCASTER_H
