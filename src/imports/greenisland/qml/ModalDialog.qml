/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
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
import FluidCore 1.0
import FluidUi 1.0

Item {
    property alias title: titleBar.children
	property alias content: contentItem.children
    property alias buttons: buttonsItem.children

    property int status: DialogStatus.Closed

    signal accepted
    signal rejected
    signal clickedOutside

    anchors.fill: parent
    z: 9000
    visible: status == DialogStatus.Open

	Rectangle {
		id: overlay
        anchors.fill: parent
		color: "black"
        opacity: parent.visible ? 0.75 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: clickedOutside()
        }
	}

	FrameSvgItem {
		id: frame
		anchors {
			horizontalCenter: overlay.horizontalCenter
			verticalCenter: overlay.verticalCenter
		}
		imagePath: "dialogs/background"
        width: Math.max(titleBar.childrenRect.width, contentItem.childrenRect.width, buttonsItem.childrenRect.width) + margins.left + margins.right
        height: titleBar.height + contentItem.childrenRect.height + buttonsItem.height + margins.top + margins.bottom
	}

	Item {
		anchors {
			fill: frame
			leftMargin: frame.margins.left
			topMargin: frame.margins.top
			rightMargin: frame.margins.right
			bottomMargin: frame.margins.bottom
		}

        // Consume all key events that are not processed by children
        Keys.onPressed: event.accepted = true
        Keys.onReleased: event.accepted = true

        Item {
            id: titleBar
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }
            height: children.length > 0 && children[0].visible ? childrenRect.height : 0
        }

        Item {
            id: contentItem
            anchors {
                left: parent.left
                top: titleBar.bottom
                right: parent.right
                bottom: buttonsItem.top
                topMargin: titleBar.height + 6
                bottomMargin: 6
            }
            height: childrenRect.height
        }

        Item {
            id: buttonsItem
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                topMargin: 6
                bottomMargin: 4
            }
            height: childrenRect.height
        }
	}

    function open() {
        if (status == DialogStatus.Opening || status == DialogStatus.Open)
            return;
        status = DialogStatus.Open;
    }

    function close() {
        if (status == DialogStatus.Closing || status == DialogStatus.Closed)
            return;
        status = DialogStatus.Closed;
    }

    function accept() {
        if (status == DialogStatus.Open) {
            close();
            accepted();
        }
    }

    function reject() {
        if (status == DialogStatus.Open) {
            close();
            rejected();
        }
    }
}
