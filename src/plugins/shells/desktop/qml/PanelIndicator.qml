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

Item {
    id: indicator

    // Label text and icon name
    property string label
    property string iconName

    // Menu object
    property variant menu

    // Icon size
    property real iconSize: theme.smallIconSize

    // Spacing between icon and label
    property real spacing: iconSize / 4

    // PanelView objectName
    property variant panelView

    width: iconItem.width + labelItem.paintedWidth + (spacing * 4)

    QtObject {
        id: internal

        property bool pressed: false
        property bool hovered: false
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            if (panelView.isMenuTriggered())
                panelView.showMenu(indicator);

            internal.hovered = true;
        }
        onExited: {
            if (panelView.isMenuTriggered()) {
                panelView.hideMenu();
                internal.pressed = false;
            }

            internal.hovered = false;
        }
        onClicked: {
            internal.pressed = !internal.pressed;

            if (internal.pressed)
                panelView.showMenu(indicator);
            else
                panelView.closeMenu();
        }
    }

    /*
    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            // No fun without an indicator!
            if (!indicator)
                return;

            // Indicators must have a "decorator" property which we set to
            // the decoration object the first time it's created
            if (!("decorator" in indicator.item))
                return;

            // Reuse existing decoration if available
            if (indicator.item.decorator) {
                indicator.item.decorator.x = root.x;
                indicator.item.decorator.y = panel.panelHeight;
            } else {
                // Create a decoration for the indicator's item
                var itemDecorationComponent = Qt.createComponent("PanelIndicatorDecoration.qml");
                indicator.item.decorator = itemDecorationComponent.createObject(root, {"x": root.x, "y": panel.panelHeight});

                // Decorate the indicator's item
                indicator.item.parent = indicator.item.decorator.children[0];
                indicator.item.x = indicator.item.y = 0;
            }

            // Avoid indicator's item overflow
            // FIXME: This doesn't work because width and height can't be set
            if (indicator.item.width > indicator.item.decorator.width)
                indicator.item.width = indicator.item.decorator.width;
            if (indicator.item.height > indicator.item.decorator.height)
                indicator.item.height = indicator.item.decorator.height;

            // Show or hide the indicator's item
            checked = !checked;
            indicator.item.decorator.visible = checked;
        }
    }
    */

    Rectangle {
        id: highlight
        anchors.fill: parent
        color: internal.hovered || internal.pressed ? theme.highlightColor : "transparent"

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
            color: internal.hovered || internal.pressed ? theme.highlightedTextColor : theme.windowTextColor
            visible: label != ""
        }
    }

    function showMenu(mouse) {
        menu.parent = indicator.parent;
        menu.x = indicator.x;
        menu.y = indicator.parent.height + indicator.y;
        menu.visible = true;
    }

    function hideMenu() {
        menu.visible = false;
    }
}
