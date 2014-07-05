/****************************************************************************
 * This file is part of Hawaii Shell.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
import GreenIsland 1.0
import GreenIsland.Core 1.0

Item {
    property alias layers: layers

    id: root

    Timer {
        id: idleTimer
        interval: compositor.idleInterval
        onIntervalChanged: {
            if (running)
                restart();
        }
    }

    ListModel {
        id: surfaceModel
    }

    function surfaceMapped(surface) {
        // Assume application window role by default
        if (typeof(surface.windowProperties.role) == "undefined")
            surface.windowProperties.role = Compositor.ApplicationRole;

        if (surface.windowProperties.role === Compositor.ApplicationRole) {
            console.debug("Surface " + surface + " mapped (" +
                          "className: \"" + surface.className + "\", " +
                          "title: \"" + surface.title + "\"): " +
                          surface.size.width + "x" + surface.size.height + " @ " +
                          surface.pos.x + "x" + surface.pos.y);
        } else {
            console.debug("Surface " + surface + " mapped (role: " +
                          surface.windowProperties.role + "): " +
                          surface.size.width + "x" + surface.size.height + " @ " +
                          surface.pos.x + "x" + surface.pos.y);
        }

        // Create surface item
        var component = Qt.createComponent("WaylandWindow.qml");
        var window = component.createObject(compositor, {surface: surface});
        console.error(component.errorString());

        // Add surface to the model
        surfaceModel.append({"surface": surface, "window": window});
    }

    function surfaceUnmapped(surface) {
        if (surface.windowProperties.role === Compositor.ApplicationRole) {
            console.debug("Surface " + surface + " unmapped (" +
                          "className: \"" + surface.className + "\", " +
                          "title: \"" + surface.title + "\")");
        } else {
            console.debug("Surface " + surface + " mapped (role: " +
                          surface.windowProperties.role + "): " +
                          surface.size.width + "x" + surface.size.height + " @ " +
                          surface.pos.x + "x" + surface.pos.y);
        }

        // Remove surface from model
        var i;
        for (i = 0; i < surfaceModel.count; i++) {
            var entry = surfaceModel.get(i);

            if (entry.surface === surface) {
                entry.window.destroy();
                surfaceModel.remove(i, 1);
                break;
            }
        }

        // Damage all surfaces
        //damageAll();
    }

    Connections {
        target: compositor
        onIdleInhibitResetRequested: compositor.idleInhibit = 0
        onIdleTimerStartRequested: idleTimer.running = true
        onIdleTimerStopRequested: idleTimer.running = false
        onIdle: {
            // Fade the desktop out
            splash.opacity = 1.0;

            // Lock the session
            compositor.lockSession();
        }
        onWake: {
            // Unlock the session
            compositor.unlockSession();
        }
        onFadeIn: {
            // Fade the desktop in
            splash.opacity = 0.0;

            // Damage all surfaces
            //damageAll();
        }
        onFadeOut: {
            // Fade the desktop out
            splash.opacity = 1.0;
        }
        onUnlocked: {
            // Fade the desktop in
            splash.opacity = 0.0;

            // Damage all surfaces
            //damageAll();
        }
        onReady: {
            // Fade the desktop in
            splash.opacity = 0.0;

            // Start idle timer
            idleTimer.running = true
        }
        onWorkspaceAdded: {
            // Add a new Workspaces
            console.debug("Add a new workspace");
            layers.workspaces.addWorkspace();
        }
    }

    /*
     * Components
     */

    Image {
        anchors.fill: parent
        source: "../images/wallpaper.png"
        fillMode: Image.Tile
    }

    Rectangle {
        color: "red"
        x: 10
        y: 10
        width: 150
        height: 150
        z: 900000

        MouseArea {
            anchors.fill: parent
            preventStealing: true
            onClicked: {
                console.log("!!!");
                root.layers.workspaces.selectNextWorkspace();
            }
        }
    }

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

    // Black rectangle for fade-in and fade-out effects
    Rectangle {
        id: splash
        anchors.fill: parent
        color: "black"
        z: 999

        Behavior on opacity {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 250
            }
        }
    }

    // Layers for windows
    Layers {
        id: layers
        anchors.fill: parent
        z: 998
    }
}
