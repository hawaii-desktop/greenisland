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
import "PanelMenuManager.js" as PanelMenuManager

Item {
    id: indicator

    // Label text and icon name
    property string label
    property string iconName

    // Menu object
    property PanelMenu menu

    // Icon size
    property real iconSize: theme.smallIconSize

    // Spacing between icon and label
    property real spacing: iconSize / 4

    // Selected and hovered
    property bool selected: false
    property bool hovered: false

    width: iconItem.width + labelItem.paintedWidth + (spacing * 4)

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            hovered = menu != null;

            if (menu && PanelMenuManager.triggered) {
                if (PanelMenuManager.currentIndicator) {
                    if (PanelMenuManager.currentIndicator.menu)
                        PanelMenuManager.currentIndicator.menu.close();

                    PanelMenuManager.currentIndicator.selected = false;
                }

                PanelMenuManager.currentIndicator = indicator;
                selected = true;
                menu.open();
            }
        }
        onExited: {
            hovered = false;
        }
        onClicked: {
            if (!menu)
                return;

            selected = !selected;

            if (selected)
                menu.open();
            else
                menu.close();

            PanelMenuManager.triggered = selected;
            PanelMenuManager.currentIndicator = indicator;
        }
    }

    onMenuChanged: {
        if (menu)
            menu.parent = indicator;
    }

    FrameSvgItem {
        id: highlight
        anchors.fill: parent
        imagePath: "widgets/menuitem"
        prefix: hovered || selected ? "selected" : ""
        opacity: hovered || selected ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    Item {
        anchors {
            fill: highlight
            leftMargin: highlight.margins.left
            topMargin: highlight.margins.top
            rightMargin: highlight.margins.right
            bottomMargin: highlight.margins.bottom
        }

        Image {
            id: iconItem
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                leftMargin: spacing
                rightMargin: spacing
            }
            source: "image://desktoptheme/" + (iconName ? iconName : "unknown")
            sourceSize: Qt.size(iconSize, iconSize)
            width: iconSize
            height: iconSize
            smooth: true
            fillMode: Image.PreserveAspectFit
            visible: iconName != ""
        }

        Label {
            id: labelItem
            anchors {
                left: iconItem.right
                verticalCenter: iconItem.verticalCenter
                leftMargin: spacing
                rightMargin: spacing
            }
            text: label
            // TODO: Define a specific font in Fluid::Theme and use it here
            font.weight: Font.Bold
            font.pointSize: 9
            color: hovered || selected ? theme.highlightedTextColor : theme.windowTextColor
            visible: label != ""
        }
    }
}
