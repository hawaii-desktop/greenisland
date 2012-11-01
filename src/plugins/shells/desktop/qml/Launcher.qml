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
import GreenIsland 1.0
import FluidCore 1.0

Item {
    id: launcherContainer

    // TODO: Define margins and padding in Fluid::Theme
    property real padding: 8

    // Size
    property real launcherSize: 0

    // Tile size
    property alias tileSize: launcher.tileSize

    // Alignment
    // TODO: From settings
    property int alignment: LauncherAlignment.Bottom

    // Number of items
    property alias count: launcher.count

    FrameSvgItem {
        id: frame
        anchors.fill: parent
        enabledBorders: {
            switch (alignment) {
            case LauncherAlignment.Left:
                return FrameSvgItem.RightBorder;
            case LauncherAlignment.Right:
                return FrameSvgItem.LeftBorder;
            case LauncherAlignment.Bottom:
                return FrameSvgItem.TopBorder;
            }
        }
        imagePath: "widgets/background"
    }

    LauncherView {
        id: launcher
        anchors.fill: frame
    }

    states: [
        State {
            name: "left"
            when: alignment == LauncherAlignment.Left

            PropertyChanges {
                target: launcherContainer
                launcherSize: tileSize + frame.margins.right + (padding * 2)
            }
            AnchorChanges {
                target: launcher
                anchors.horizontalCenter: frame.horizontalCenter
            }
            PropertyChanges {
                target: launcher
                anchors.leftMargin: padding
                anchors.topMargin: padding
                anchors.rightMargin: frame.margins.right + padding
                anchors.bottomMargin: padding
                orientation: ListView.Vertical
            }
        },
        State {
            name: "right"
            when: alignment == LauncherAlignment.Right

            PropertyChanges {
                target: launcherContainer
                launcherSize: tileSize + frame.margins.left + (padding * 2)
            }
            AnchorChanges {
                target: launcher
                anchors.horizontalCenter: frame.horizontalCenter
            }
            PropertyChanges {
                target: launcher
                anchors.leftMargin: frame.margins.left + padding
                anchors.topMargin: padding
                anchors.rightMargin: padding
                anchors.bottomMargin: padding
                orientation: ListView.Vertical
            }
        },
        State {
            name: "bottom"
            when: alignment == LauncherAlignment.Bottom

            PropertyChanges {
                target: launcherContainer
                launcherSize: tileSize + frame.margins.top + (padding * 2)
            }
            AnchorChanges {
                target: launcher
                anchors.verticalCenter: frame.verticalCenter
            }
            PropertyChanges {
                target: launcher
                anchors.leftMargin: padding
                anchors.topMargin: frame.margins.top + padding
                anchors.rightMargin: padding
                anchors.bottomMargin: padding
                orientation: ListView.Horizontal
            }
        }
    ]

    // Animate width when the launcher is created (when aligned left or right)
    NumberAnimation on width {
        running: opacity > 0 && (alignment == LauncherAlignment.Left || alignment == LauncherAlignment.Right)
        easing.type: Easing.InQuad
        duration: 400
        from: 0
        to: launcherSize
    }

    // Animate height when the launcher is created (when laid down to the bottom)
    NumberAnimation on height {
        running: opacity > 0 && alignment == LauncherAlignment.Bottom
        easing.type: Easing.InQuad
        duration: 400
        from: 0
        to: launcherSize
    }
}
