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
    property real padding: 4

    // Size
    property real launcherSize: 0

    // Tile size
    property alias tileSize: launcher.tileSize

    // Alignment and orientation
    property int alignment: LauncherAlignment.Bottom
    property alias orientation: launcher.orientation

    // Number of items
    property alias count: launcher.count

    // AppChooser object
    property var appChooser: AppChooser {}

    Settings {
        id: settings
        schema: "org.hawaii.greenisland"
        group: "launcher"
        onValueChanged: {
            var val = settings.value("alignment");
            if (val === "left")
                alignment = LauncherAlignment.Left;
            else if (val === "right")
                alignment = LauncherAlignment.Right;
            else
                alignment = LauncherAlignment.Bottom;
        }
    }

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
                return FrameSvgItem.LeftBorder | FrameSvgItem.TopBorder | FrameSvgItem.RightBorder;
            }
        }
        imagePath: "widgets/panel-background"
        prefix: "south-mini"
    }

    LauncherView {
        id: launcher
        anchors.fill: frame
        orientation: {
            switch (alignment) {
            case LauncherAlignment.Bottom:
                return ListView.Horizontal;
            default:
                return ListView.Vertical;
            }
        }
    }

    states: [
        State {
            name: "left"
            when: alignment === LauncherAlignment.Left

            PropertyChanges {
                target: launcherContainer
                launcherSize: tileSize + frame.margins.right + (padding * 2)
            }
            PropertyChanges {
                target: launcher
                anchors.rightMargin: frame.margins.right + padding
                orientation: ListView.Vertical
            }
        },
        State {
            name: "right"
            when: alignment === LauncherAlignment.Right

            PropertyChanges {
                target: launcherContainer
                launcherSize: tileSize + frame.margins.left + (padding * 2)
            }
            PropertyChanges {
                target: launcher
                anchors.leftMargin: frame.margins.left + padding
                orientation: ListView.Vertical
            }
        },
        State {
            name: "bottom"
            when: alignment === LauncherAlignment.Bottom

            PropertyChanges {
                target: launcherContainer
                launcherSize: tileSize + frame.margins.top + (padding * 2)
            }
            PropertyChanges {
                target: launcher
                anchors.topMargin: frame.margins.top + padding
                orientation: ListView.Horizontal
            }
        }
    ]
}
