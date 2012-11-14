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

    x: shell.screenGeometry.x
    y: shell.screenGeometry.y
    width: shell.screenGeometry.width
    height: shell.screenGeometry.height

    // Panel
    Loader {
        id: panelComponent
        anchors { top: parent.top; left: parent.left; right: parent.right }
        z: 2
        source: "Panel.qml"
        asynchronous: true
        onLoaded: calculateGeometry()

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

            // Set a pointer to the AppChooser object
            launcherComponent.item.appChooserObject = appChooser;
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
                    y: panelComponent.item.height
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
                    y: panelComponent.item.height
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
        shell.availableGeometry = Qt.rect(x, y, width, height);

        // ...unless the panel is loaded
        if (panelComponent.status == Loader.Ready) {
            panelComponent.height = panelComponent.item.panelHeight;
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
                shell.availableGeometry.x = shell.availableGeometry.width - launcherComponent.item.launcherSize;
                shell.availableGeometry.width -= launcherComponent.item.launcherSize;
                break;
            case LauncherAlignment.Bottom:
                shell.availableGeometry.height -= launcherComponent.item.launcherSize;
                break;
            }
        }

        // Resize AppChooser
        appChooser.width = shell.availableGeometry.width / 1.1;
        appChooser.height = shell.availableGeometry.height / 1.1;
    }
}
