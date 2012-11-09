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
import FluidCore 1.0
import FluidUi 1.0

Item {
    id: root

    //default property alias content: menu.content
    default property alias content: menu.children
    property int status: DialogStatus.Closed

    property real paddingLarge: 16

    width: menuContainer.width
    height: menuContainer.height
    state: "hidden"
    enabled: status == DialogStatus.Open
    visible: status != DialogStatus.Closed

    FrameSvgItem {
        id: menuContainer
        width: menu.width
        height: menu.height
        imagePath: "widgets/menu-background"

        PanelMenuContent {
            id: menu
            anchors {
                left: parent.left
                top: parent.top
                leftMargin: menuContainer.margins.left
                topMargin: menuContainer.margins.top
                rightMargin: menuContainer.margins.right
            }
            onItemClicked: close()
            width: 500
            height: 500
        }
    }

    states: [
        State {
            name: "visible"
            when: status == DialogStatus.Opening || status == DialogStatus.Open
            PropertyChanges {
                target: menuContainer
                x: root.parent.x
                y: root.parent.parent.height + root.parent.y
                opacity: 1
            }
        },
        State {
            name: "hidden"
            when: status == DialogStatus.Closing || status == DialogStatus.Closed
            PropertyChanges {
                target: menuContainer
                x: root.parent.x
                y: 0
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "visible"
            to: "hidden"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        target: menuContainer
                        property: "y"
                        duration: 50
                        easing.type: Easing.Linear
                    }
                    NumberAnimation {
                        target: menuContainer
                        property: "opacity"
                        duration: 200
                        easing.type: Easing.Linear
                    }
                }
                PropertyAction { target: root; property: "status"; value: DialogStatus.Closed }
            }
        },
        Transition {
            from: "hidden"
            to: "visible"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        target: menuContainer
                        property: "y"
                        duration: 100
                        easing.type: Easing.Linear
                    }
                    NumberAnimation {
                        target: menuContainer
                        property: "opacity"
                        duration: 250
                        easing.type: Easing.Linear
                    }
                }
                PropertyAction { target: root; property: "status"; value: DialogStatus.Open }
            }
        }
    ]

    function open() {
        if (status == DialogStatus.Open || status == DialogStatus.Opening)
            return;

        status = DialogStatus.Opening;
    }

    function close() {
        if (status == DialogStatus.Closed)
            return;

        status = DialogStatus.Closing;
    }
}
