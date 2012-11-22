/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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
import GreenIsland 1.0
import FluidCore 1.0

Item {
    id: root

    x: quickview.screenGeometry.x
    y: quickview.screenGeometry.y
    width: quickview.screenGeometry.width
    height: quickview.screenGeometry.height

    // Recalculate geometry when the size changes
    onWidthChanged: calculateGeometry()
    onHeightChanged: calculateGeometry()

    // Panel
    Panel {
        id: panel
        anchors { top: parent.top; left: parent.left; right: parent.right }
        z: 2

        // Animate Panel when it shows up
        Behavior on height {
            NumberAnimation { easing.type: Easing.InQuad; duration: 750 }
        }
    }

    // Launcher
    Loader {
        id: launcherComponent
        z: 2
        source: "Launcher.qml"
        asynchronous: true
        onLoaded: {
            // Recalculate geometry
            calculateGeometry();
        }

        // Animate Launcher when it shows up
        Behavior on height {
            NumberAnimation { easing.type: Easing.InQuad; duration: 750 }
        }

        states: [
            State {
                name: "left"
                when: launcherComponent.item.alignment === LauncherAlignment.Left

                AnchorChanges {
                    target: launcherComponent
                    anchors.left: root.left
                }
                PropertyChanges {
                    target: launcherComponent
                    y: panel.height
                    width: item.launcherSize
                    height: root.height - y
                }
            },
            State {
                name: "right"
                when: launcherComponent.item.alignment === LauncherAlignment.Right

                AnchorChanges {
                    target: launcherComponent
                    anchors.right: root.right
                }
                PropertyChanges {
                    target: launcherComponent
                    y: panel.height
                    width: item.launcherSize
                    height: root.height - y
                }
            },
            State {
                name: "bottom"
                when: launcherComponent.item.alignment === LauncherAlignment.Bottom

                AnchorChanges {
                    target: launcherComponent
                    anchors.bottom: root.bottom
                    anchors.horizontalCenter: root.horizontalCenter
                }
                PropertyChanges {
                    target: launcherComponent
                    width: root.width
                    height: item.launcherSize
                }
            }
        ]
    }

    // Application chooser
    AppChooser {
        id: appChooser
        z: 3
        visible: false
    }

    Component.onCompleted: calculateGeometry()

    // Top-right corner is sensible and exposes all the windows
/*
    MouseArea {
        x: root.width - width
        y: 0
        width: panelComponent.height
        height: panelComponent.height
        hoverEnabled: true
        onEntered: {
            root.normalLayout = !root.normalLayout;
            CompositorLogic.relayout();
        }
    }
*/

    /*
    MouseArea {
        x: shell.availableGeometry.x
        y: shell.availableGeometry.y
        width: shell.availableGeometry.width
        height: shell.availableGeometry.height
        hoverEnabled: true
        onEntered: {
            console.log("ciao");
        }
    }
    */

    function calculateGeometry() {
        // Available geometry equals screen geometry
        var geometry = Qt.rect(x, y, width, height); //quickview.screenGeometry;

        // ...unless the panel is loaded
        geometry.y = panel.height;
        geometry.height -= panel.height;

        // ...or the launcher is
        if (launcherComponent.status == Loader.Ready) {
            switch (launcherComponent.item.alignment) {
            case LauncherAlignment.Left:
                geometry.x = launcherComponent.item.launcherSize;
                geometry.width -= launcherComponent.item.launcherSize;
                break;
            case LauncherAlignment.Right:
                geometry.x = geometry.width - launcherComponent.item.launcherSize;
                geometry.width -= launcherComponent.item.launcherSize;
                break;
            case LauncherAlignment.Bottom:
                geometry.height -= launcherComponent.item.launcherSize;
                break;
            }
        }

        // Set the available geometry to the result of the above computation
        quickview.availableGeometry = geometry;
    }
}
