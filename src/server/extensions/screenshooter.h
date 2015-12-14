/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef GREENISLAND_SCREENSHOOTER_H
#define GREENISLAND_SCREENSHOOTER_H

#include <QtCore/QLoggingCategory>

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

#include <GreenIsland/server/greenislandserver_export.h>

Q_DECLARE_LOGGING_CATEGORY(SCREENSHOOTER_PROTOCOL)

namespace GreenIsland {

namespace Server {

class ScreenshooterPrivate;

class GREENISLANDSERVER_EXPORT Screenshooter : public QWaylandExtensionTemplate<Screenshooter>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screenshooter)
public:
    Screenshooter();
    Screenshooter(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void done();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENSHOOTER_H
