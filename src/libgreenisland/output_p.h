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

#ifndef GREENISLAND_OUTPUT_P_H
#define GREENISLAND_OUTPUT_P_H

#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/private/qobject_p.h>

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include "output.h"

#include "qwayland-server-wayland.h"

namespace GreenIsland {

class OutputPrivate : public QObjectPrivate, public QtWaylandServer::wl_output
{
    Q_DECLARE_PUBLIC(Output)
public:
    OutputPrivate();

    void sendGeometryInfo(Resource *resource, bool sendDone = false);
    void sendAllGeometryInfo(bool sendDone = false);

    void sendScaleFactor(Resource *resource, bool sendDone = false);
    void sendAllScaleFactor(bool sendDone);

    void sendModes(Resource *resource, bool sendDone = false);
    void sendAllModes(bool sendDone = false);

    void sendMode(Resource *resource, const Output::Mode &mode);

    void setPrimary(bool value);

    void update();

    void _q_updateStarted();

    QString name;
    QString manufacturer;
    QString model;
    int id;
    bool primary;
    QPoint position;
    QList<Output::Mode> modes;
    QRect availableGeometry;
    QSize physicalSize;
    Output::Subpixel subpixel;
    Output::Transform transform;
    int scaleFactor;
    OutputWindow *window;
    bool updateScheduled;
    QSize hotSpotSize;
    quint64 hotSpotThreshold;
    quint64 hotSpotPushTime;

private:
    void output_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
};

} // namespace GreenIsland

#endif // GREENISLAND_OUTPUT_P_H

