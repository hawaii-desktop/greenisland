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
