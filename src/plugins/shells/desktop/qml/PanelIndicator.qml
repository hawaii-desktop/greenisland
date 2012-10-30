/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL$
 *
 * Desktop Shell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Desktop Shell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Desktop Shell.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

import QtQuick 2.0
import FluidCore 1.0
import FluidUi 1.0

Item {
    // Label text and icon name from the C++ indicator
    property string label: null
    property string iconName: null

    // Is it checked?
    property bool checked: false

    width: row.width
    height: row.height

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (!indicator)
                return;

            checked = !checked;
            if (checked)
                indicator.showView();
            else
                indicator.hideView();
        }
    }

    Row {
        id: row
        // TODO: Load from Fluid::Theme
        spacing: 4

        Image {
            id: iconItem
            anchors.verticalCenter: parent.verticalCenter
            asynchronous: true
            source: "image://desktoptheme/" + (iconName ? iconName : "unknown")
            sourceSize.width: theme.smallMediumIconSize
            sourceSize.height: theme.smallMediumIconSize
            visible: iconName
        }

        Label {
            id: labelItem
            anchors.verticalCenter: iconItem.verticalCenter
            text: label
            // TODO: Define a specific font in Fluid::Theme and use it here
            font.weight: Font.Bold
            color: theme.brightTextColor
        }
    }
}
