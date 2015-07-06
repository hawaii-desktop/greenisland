/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *               2015 Michael Spencer <sonrisesoftware@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Michael Spencer
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

import QtQuick 2.4
import QtQuick.Controls 1.2
import QtCompositor 1.0
import GreenIsland 1.0
import GreenIsland.Desktop 1.0
import "overlays"

Rectangle {

    Image {
        anchors.fill: parent
        source: "images/wallpaper.png"
        fillMode: Image.Tile
    }

    ToolBar {
        id: toolbar
        Row {
            spacing: 10

            anchors.verticalCenter: parent.verticalCenter

            Button {
                text: "Switcher"
                onClicked: switcher.visible = !switcher.visible
                enabled: windowManager.windows.count > 0
                anchors.verticalCenter: parent.verticalCenter
            }

            Repeater {
                model: windowManager.windows
                delegate: ToolButton {
                    text: window.title
                    anchors.verticalCenter: parent.verticalCenter
                    checked: windowManager.activeWindow == item
                    checkable: true

                    onCheckedChanged: {
                        if (checked) {
                            windowManager.moveFront(item)
                        }

                        checked = Qt.binding(function() {
                            return windowManager.activeWindow == item
                        })
                    }
                }
            }
        }

        Row {
            spacing: 10

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right

            Repeater {
                model: windowManager.workspaces
                delegate: ToolButton {
                    text: index + 1
                    anchors.verticalCenter: parent.verticalCenter
                    checked: windowManager.currentWorkspace == workspace
                    checkable: true

                    onCheckedChanged: {
                        if (checked) {
                            windowManager.currentWorkspace = workspace
                        }

                        checked = Qt.binding(function() {
                            return windowManager.currentWorkspace == workspace
                        })
                    }
                }
            }
        }
    }

    WindowManager {
        id: windowManager
        anchors {
            left: parent.left
            right: parent.right
            top: toolbar.bottom
            bottom: parent.bottom
        }

        topLevelWindowComponent: TopLevelWindow {
            id: window

            unresponsiveEffectComponent: UnresponsiveOverlay {}

            onMinimize: {
                x = Qt.binding(function() { return (windowManager.width - window.width)/2 })
                y = 0
                scale = 0
                opacity = 0
            }

            animation: WindowAnimation {
                mapAnimation: NumberAnimation {
                    target: window
                    property: "opacity"
                    from: 0; to: 1
                    duration: 250
                }

                unmapAnimation: ParallelAnimation {
                    NumberAnimation {
                        target: window
                        property: "opacity"
                        from: 1; to: 0
                        duration: 250
                    }
                    NumberAnimation {
                        target: window
                        property: "scale"
                        from: 1; to: 0.5
                        duration: 250
                    }
                }
            }
        }

        Workspace {
            id: workspace1
            isCurrentWorkspace: windowManager.currentWorkspace == workspace1
            visible: isCurrentWorkspace
        }

        Workspace {
            id: workspace2
            isCurrentWorkspace: windowManager.currentWorkspace == workspace2
            visible: isCurrentWorkspace
        }
    }

    Rectangle {
        id: switcher
        anchors.centerIn: parent
        color: Qt.rgba(0,0,0,0.7)
        radius: 5
        height: row.height + 20
        width: row.width + 20
        visible: false

        Row {
            id: row
            anchors.centerIn: parent
            spacing: 10

            Repeater {
                model: windowManager.orderedWindows
                delegate: Rectangle {
                    height: 100
                    width: preview.implicitWidth

                    color: "transparent"

                    border.color: preview.isActive ? "white" : "transparent"
                    border.width: 2
                    radius: 2

                    WindowPreview {
                        id: preview
                        anchors {
                            fill: parent
                            margins: 5
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.AllButtons
                        onClicked: preview.activate()
                    }
                }
            }
        }
    }
}
