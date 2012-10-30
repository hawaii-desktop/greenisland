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
import FluidUi 1.0
import GreenIsland 1.0

Rectangle {
    id: appchooser

    color: "black"
    opacity: 0.90

    border {
        color: "#f1f1f1"
        width: 4
    }
    radius: 6

    GridView {
        id: grid

        anchors.fill: parent
        anchors.margins: 4

        cacheBuffer: 1000
        cellWidth: 128
        cellHeight: 128

        displaced: Transition {
            NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad }
        }

        model: VisualDataModel {
            id: visualModel

            model: AvailableApplicationsModel {}
            delegate: AppChooserDelegate {
                visualIndex: VisualDataModel.itemsIndex
                icon: "image://desktoptheme/" + (iconName ? iconName : "unknown")
                label: name
            }
        }
    }
}
