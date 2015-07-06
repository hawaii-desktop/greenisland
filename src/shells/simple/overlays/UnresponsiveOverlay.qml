/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: root

    color: Qt.rgba(0,0,0,0.3)

    ColumnLayout {
        anchors.fill: parent
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
                onClicked: window.kill()
            }

            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
