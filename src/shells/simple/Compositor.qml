/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

import QtQuick 2.0
import QtCompositor 1.0
import GreenIsland 1.0 as GreenIsland
import "WindowManagement.js" as WindowManagement

Item {
    readonly property alias screenView: screenView
    readonly property alias surfaceModel: surfaceModel
    property var activeWindow: null
    readonly property int activeWindowIndex: WindowManagement.getActiveWindowIndex()
    readonly property var windowList: WindowManagement.windowList

    id: compositorRoot

    ListModel {
        id: surfaceModel
    }

    Connections {
        target: GreenIsland.Compositor
        onWindowMapped: {
            // A window was mapped
            WindowManagement.windowMapped(window);
        }
        onWindowUnmapped: {
            // A window was unmapped
            WindowManagement.windowUnmapped(window);
        }
        onWindowDestroyed: {
            // A window was unmapped
            WindowManagement.windowDestroyed(id);
        }
    }

    /*
     * Components
     */

    // FPS counter
    Text {
        anchors {
            top: parent.top
            right: parent.right
        }
        z: 1000
        text: fpsCounter.fps
        font.pointSize: 36
        style: Text.Raised
        styleColor: "#222"
        color: "white"
        visible: false

        FpsCounter {
            id: fpsCounter
        }
    }

    // Screen
    ScreenView {
        id: screenView
        anchors.fill: parent
        z: 998
    }

    /*
     * Methods
     */

    function moveFront(window) {
        return WindowManagement.moveFront(window);
    }

    function enableInput() {
        var i;
        for (i = 0; i < compositorRoot.surfaceModel.count; i++) {
            var window = compositorRoot.surfaceModel.get(i).item;
            window.child.focus = true;
        }
    }

    function disableInput() {
        var i;
        for (i = 0; i < compositorRoot.surfaceModel.count; i++) {
            var window = compositorRoot.surfaceModel.get(i).item;
            window.child.focus = false;
        }
    }
}
