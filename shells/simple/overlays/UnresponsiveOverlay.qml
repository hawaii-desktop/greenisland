/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

Colorize {
    property alias window: root.source

    id: root
    hue: 0.0
    saturation: 0.5
    lightness: -0.5
    z: 1

    Behavior on opacity {
        NumberAnimation {
            easing.type: Easing.Linear
            duration: 250
        }
    }

    // Automatically ping the surface
    Timer {
        running: window.unresponsive
        interval: 10000
        onTriggered: window.pingSurface()
    }

    // Block input on the surface
    MouseArea {
        anchors.fill: parent
        enabled: window.unresponsive
        cursorShape: Qt.BusyCursor
        z: 1
    }

    ColumnLayout {
        anchors.fill: parent
        enabled: window.unresponsive
        z: 2

        Item {
            Layout.fillHeight: true
        }

        Text {
            font.bold: true
            font.pointSize: 14
            color: "#f1f1f1"
            text: qsTr("Application is not responding")
            horizontalAlignment: Qt.AlignHCenter

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
        }

        Text {
            font.pointSize: 10
            color: "#f1f1f1"
            text: qsTr("This application is unresponsive, you may choose to wait " +
                       "a short while for it to continue or force the application to " +
                       "quit entirely.")
            horizontalAlignment: Qt.AlignHCenter
            wrapMode: Text.Wrap

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }

        RowLayout {
            Button {
                text: qsTr("Wait")
                onClicked: window.pingSurface()
            }

            Button {
                text: qsTr("Force Quit")
                onClicked: window.child.surface.client.kill()
            }

            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
