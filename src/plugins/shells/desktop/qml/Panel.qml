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

Item {
    // Panel height
    property real panelHeight: 28 + frame.margins.bottom

    Behavior on height {
        NumberAnimation { easing.type: Easing.InQuad; duration: 400 }
    }

    FrameSvgItem {
        id: frame
        anchors.fill: parent
        enabledBorders: FrameSvgItem.BottomBorder
        imagePath: "widgets/translucentbackground"
    }

    PanelView {
        anchors.fill: frame
        anchors.bottomMargin: frame.margins.bottom
    }

    // Animate height when the panel is created
    NumberAnimation on height {
        easing.type: Easing.InQuad
        duration: 400
        to: panelHeight
    }
}
