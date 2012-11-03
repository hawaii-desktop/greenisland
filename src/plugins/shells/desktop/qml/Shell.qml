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
import QtGraphicalEffects 1.0
import GreenIsland 1.0
import "CompositorLogic.js" as CompositorLogic

Item {
    id: root

    // Screen geometry
    property rect geometry

    // Currently selected window
    property variant selectedWindow: null

    // True if we're using the normal layout
    property bool normalLayout: true

    // Recalculate geometry when the size changes
    onWidthChanged: calculateGeometry()
    onHeightChanged: calculateGeometry()

    // Opacity will be set to 1.0 by the fade-in animation
    opacity: 0.0

    // Fade-in animation
    NumberAnimation on opacity {
        easing.type: Easing.Linear
        duration: 1500
        to: 1.0
    }

    // Desktop wallpaper
    Image {
        id: background
        anchors.fill: parent
        fillMode: Image.Tile
        // TODO: From settings
        source: "file:///opt/hawaii/share/wallpapers/Nature/Finally_Summer_in_Germany/contents/images/1920x1200.jpg"
        smooth: true

        Behavior on opacity {
            NumberAnimation { easing.type: Easing.Linear; duration: 500 }
        }
    }

    // Panel
    Loader {
        id: panelComponent
        anchors { top: parent.top; left: parent.left; right: parent.right }
        z: 2
        source: "Panel.qml"
        asynchronous: true
        onLoaded: calculateGeometry()
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

            // Set a pointer to the AppChooser object
            launcherComponent.item.appChooserObject = appChooser;
        }

        states: [
            State {
                name: "left"
                when: launcherComponent.item.alignment == LauncherAlignment.Left

                AnchorChanges {
                    target: launcherComponent
                    anchors.left: root.left
                    anchors.verticalCenter: root.verticalCenter
                }
                PropertyChanges {
                    target: launcherComponent
                    width: item.launcherSize
                    height: root.height - panelComponent.item.height
                }
            },
            State {
                name: "right"
                when: launcherComponent.item.alignment == LauncherAlignment.Right

                AnchorChanges {
                    target: launcherComponent
                    anchors.right: root.right
                    anchors.verticalCenter: root.verticalCenter
                }
                PropertyChanges {
                    target: launcherComponent
                    width: item.launcherSize
                    height: root.height - panelComponent.item.height
                }
            },
            State {
                name: "bottom"
                when: launcherComponent.item.alignment == LauncherAlignment.Bottom

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
        width: shell.availableGeometry.width / 1.1
        height: shell.availableGeometry.height / 1.1
        visible: false
    }

    // Windows loose their focus when clicking an empty spot on the desktop,
    // however this happens only if the user clicks inside the available geometry
    MouseArea {
        x: shell.availableGeometry.x
        y: shell.availableGeometry.y
        width: shell.availableGeometry.width
        height: shell.availableGeometry.height
        onClicked: {
            root.selectedWindow = null;
            root.focus = true;
        }
    }

    // Top-right corner is sensible and exposes all the windows
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

    function calculateGeometry() {
        // Available geometry equals screen geometry
        geometry = Qt.rect(x, y, width, height);
        shell.availableGeometry = geometry;

        // ...unless the panel is loaded
        if (panelComponent.status == Loader.Ready) {
            shell.availableGeometry.y += panelComponent.item.panelHeight;
            shell.availableGeometry.height -= panelComponent.item.panelHeight;
        }

        // ...or the launcher is
        if (launcherComponent.status == Loader.Ready) {
            switch (launcherComponent.item.alignment) {
            case LauncherAlignment.Left:
                shell.availableGeometry.x += launcherComponent.item.launcherSize;
                shell.availableGeometry.width -= launcherComponent.item.launcherSize;
                break;
            case LauncherAlignment.Right:
                shell.availableGeometry.x = availableGeometry.width - launcherComponent.item.launcherSize;
                shell.availableGeometry.width -= launcherComponent.item.launcherSize;
                break;
            case LauncherAlignment.Bottom:
                shell.availableGeometry.height -= launcherComponent.item.launcherSize;
                break;
            }
        }

        // Recalculate the layout
        CompositorLogic.relayout();
    }

    function windowAdded(window) {
        CompositorLogic.windowAdded(root, window);
    }

    function windowResized(window) {
        CompositorLogic.windowResized(window);
    }

    function windowDestroyed(window) {
        CompositorLogic.windowDestroyed(window);
    }

    function removeWindow(window) {
        CompositorLogic.windowRemoved(shell, window);
    }
}
