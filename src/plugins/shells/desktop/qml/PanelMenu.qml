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

    default property alias content: menuContents.children
    property int status: DialogStatus.Closed

    width: 400
    height: 400
    state: "hidden"
    enabled: status == DialogStatus.Open
    visible: status != DialogStatus.Closed

    FrameSvgItem {
        id: menuContainer
        width: menu.width
        height: menu.height
        imagePath: "widgets/menu-background"

        Item {
            id: contentItem
            x: menuContainer.margins.left
            y: menuContainer.margins.top
            width: Math.max(menuContents.width, theme.defaultFont.mSize.width * 12)
            height: Math.min(menuContents.height, theme.defaultFont.mSize.height * 25)

            Flickable {
                id: flickable
                anchors.fill: parent
                clip: true

                Column {
                    id: menuContents
                    spacing: 4
                    onChildrenChanged: {
                        // Close the popup menu when a menu item is clicked
                        connectClickedSignal();

                        // Resize the popup menu accordingly to its items
                        resizePopupMenu();
                    }
                    onChildrenRectChanged: resizePopupMenu()

                    function connectClickedSignal() {
                        for (var i = 0; i < children.length; ++i) {
                            if (children[i].clicked != undefined)
                                children[i].clicked.connect(root.close);
                        }
                    }

                    function resizePopupMenu() {
                        root.width = childrenRect.width + menuContainer.margins.left + menuContainer.margins.right;
                        root.height = childrenRect.height + menuContainer.margins.top + menuContainer.margins.bottom;
                    }
                }
            }

            ScrollBar {
                id: scrollBar
                flickableItem: flickable
                visible: flickable.contentHeight > contentItem.height
                anchors {
                    top: flickable.top
                    right: flickable.right
                }
            }
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
