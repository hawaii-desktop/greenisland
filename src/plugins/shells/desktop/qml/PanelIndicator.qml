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
    id: root

    // Top panel object
    property variant panel: null

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
