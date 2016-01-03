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

import QtQuick 2.0
import QtQuick.Window 2.2
import GreenIsland 1.0

WaylandOutput {
    default property alias data: background.data
    property alias surfacesArea: background

    id: output
    window: Window {
        property QtObject output

        id: window
        width: 1024
        height: 768
        visible: false

        WaylandMouseTracker {
            id: mouseTracker
            anchors.fill: parent
            enableWSCursor: true

            Rectangle {
                id: background
                anchors.fill: parent
                color: "#595959"
            }

            WaylandCursorItem {
                id: cursor
                inputDevice: output.compositor.defaultInputDevice
                inputEventsEnabled: false
                x: mouseTracker.mouseX - hotspotX
                y: mouseTracker.mouseY - hotspotY
            }
        }
    }
}
