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

Item {
    id: launcherContainer

    // Size
    property alias launcherSize: launcher.launcherSize

    // Tile size
    property alias tileSize: launcher.tileSize

    // Orientation
    property alias orientation: launcher.orientation

    // Number of items
    property alias count: launcher.count

    // TODO: Add an alignment (Left, Right, Bottom)

    FrameSvgItem {
        id: frame
        anchors.fill: parent
        enabledBorders: {
            // TODO: Handle alignment
            if (orientation == ListView.Horizontal)
                return FrameSvgItem.TopBorder;
            return FrameSvgItem.RightBorder;
        }
        imagePath: "widgets/background"
    }

    LauncherView {
        id: launcher
        anchors { fill: frame; margins: 4 }
    }

    // Animate width when the launcher is created
    // (only if its orientation is vertical)
    NumberAnimation on width {
        running: opacity > 0 && orientation == ListView.Vertical
        easing.type: Easing.InQuad
        duration: 400
        from: 0
        to: launcherSize
    }

    // Animate height when the launcher is created
    // (only if its orientation is horizontal)
    NumberAnimation on height {
        running: opacity > 0 && orientation == ListView.Horizontal
        easing.type: Easing.InQuad
        duration: 400
        from: 0
        to: launcherSize
    }
}
