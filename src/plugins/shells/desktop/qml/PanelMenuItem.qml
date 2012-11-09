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

    property alias text: label.text

    property real padding: 8

    signal clicked

    width: container.width
    height: container.height

    Keys.onReturnPressed: root.clicked()

    MouseArea {
        id: mousearea
        anchors.fill: container
        hoverEnabled: true
        onEntered: {
            label.color = theme.highlightedTextColor;
            container.color = theme.highlightColor;
        }
        onExited: {
            label.color = theme.windowTextColor;
            container.color = "transparent";
        }
        onClicked: root.clicked()
    }

    Rectangle {
        id: container
        anchors {
            left: parent.left
            top: parent.top
        }
        color: "transparent"
        x: 0
        y: 0
        width: label.painterWidth + 1920
        height: label.paintedHeight + (padding * 2)
        onWidthChanged: console.log("width", width)
        onHeightChanged: console.log("height", height)

        Label {
            id: label
            anchors {
                left: parent.left
                //right: parent.right
                leftMargin: padding
                rightMargin: padding
                verticalCenter: parent.verticalCenter
            }
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }
    }
}
