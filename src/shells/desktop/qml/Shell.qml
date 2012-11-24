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

    // AppChooser
    property var appChooser: appChooserObject

    // Notifications
    property var notifications: notificationsObject

    x: quickview.screenGeometry.x
    y: quickview.screenGeometry.y
    width: quickview.screenGeometry.width
    height: quickview.screenGeometry.height

    // Recalculate geometry when the size changes
    onWidthChanged: calculateGeometry()
    onHeightChanged: calculateGeometry()

    // Panel
    Loader {
        id: panelComponent
        anchors { top: parent.top; left: parent.left; right: parent.right }
        x: quickview.screenGeometry.x
        y: quickview.screenGeometry.y
        z: 2
        width: quickview.screenGeometry.width
        source: "Panel.qml"
        asynchronous: true
        onLoaded: {
            // Recalculate geometry
            calculateGeometry();
        }

        // Animate Panel when it shows up
        Behavior on height {
            NumberAnimation { easing.type: Easing.InQuad; duration: 350 }
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
            NumberAnimation { easing.type: Easing.InQuad; duration: 350 }
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

    // Active top left corner
    MouseArea {
        id: topLeftCorner
        x: compositor.screenGeometry.x
        y: compositor.screenGeometry.y
        width: 16
        height: 16
        hoverEnabled: true
        onEntered: {
            root.normalLayout = false;
            CompositorLogic.relayout()
        }
        onExited: {
            root.normalLayout = true;
            CompositorLogic.relayout();
        }
    }

    // Active top right corner
    MouseArea {
        id: topRightCorner
        x: compositor.screenGeometry.width - 16
        y: compositor.screenGeometry.y
        width: 16
        height: 16
    }

    // Active bottom left corner
    MouseArea {
        id: bottomLeftCorner
        x: compositor.screenGeometry.x
        y: compositor.screenGeometry.height - 16
        width: 16
        height: 16
    }

    // Active bottom right corner
    MouseArea {
        id: bottomRightCorner
        x: compositor.screenGeometry.width - 16
        y: compositor.screenGeometry.y
        width: 16
        height: 16
    }

    // Application chooser
    AppChooser {
        id: appChooserObject
        opacity: 0.0
        width: quickview.availableGeometry.width / 1.1
        height: quickview.availableGeometry.height / 1.1

        // Animate when it shows up
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    // Notifications
    Notifications {
        id: notificationsObject
        x: quickview.availableGeometry.width
        y: quickview.availableGeometry.y
        width: quickview.availableGeometry.width / 10
        height: quickview.availableGeometry.height

        // Animation
        Behavior on x {
            NumberAnimation { duration: 200 }
        }

        function show() {
            x = quickview.availableGeometry.width - width;
        }

        function hide() {
            x = quickview.availableGeometry.width;
        }
    }

    Component.onCompleted: calculateGeometry()

    function calculateGeometry() {
        // Available geometry equals screen geometry
        var geometry = Qt.rect(x, y, width, height); //quickview.screenGeometry;

        // ...unless the panel is loaded
        if (panelComponent.status == Loader.Ready) {
            panelComponent.height = panelComponent.item.panelHeight;
            geometry.y = panelComponent.height;
            geometry.height -= panelComponent.height;
        }

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
        notificationsObject.y = quickview.availableGeometry.y;
    }
}
