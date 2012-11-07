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

Item {
    id: root

    // Spacing between indicators
    property real spacing: 8

    QtObject {
        id: internal

        // Currently selected item
        property variant currentItem: null

        // Is a menu triggered?
        property bool triggered: false
    }

    function showMenu(item) {
        // Show the menu of the currently active indicator
        internal.currentItem = item;
        if (internal.currentItem && internal.currentItem.menu) {
            internal.currentItem.menu.parent = internal.currentItem;
            internal.currentItem.menu.x = internal.currentItem.x;
            internal.currentItem.menu.y = root.height + internal.currentItem.y;
            internal.currentItem.menu.visible = true;
            internal.triggered = true;
        }
    }

    function hideMenu() {
        if (internal.currentItem && internal.currentItem.menu)
            internal.currentItem.menu.visible = false;
    }

    function closeMenu() {
        if (internal.currentItem && internal.currentItem.menu) {
            internal.currentItem.menu.visible = false;
            internal.triggered = false;
        }
    }

    function isMenuTriggered() {
        return internal.triggered;
    }

    Item {
        id: leftBox
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width / 3

        WorkspaceIndicator {
            id: workspaceIndicator
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                rightMargin: spacing
            }
            panelView: root
        }
    }

    Item {
        id: centerBox
        anchors {
            left: leftBox.right
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width / 3

        DateIndicator {
            id: dateIndicator
            anchors {
                top: parent.top
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
                rightMargin: spacing
            }
            panelView: root
        }
    }

    Item {
        id: rightBox
        anchors {
            left: centerBox.right
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width / 3

        /*
        Row {
            id: trayArea
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }

            Repeater {
                model: IndicatorsModel {}

                PanelIndicator {
                    label: indicator.label
                    iconName: indicator.iconName
                    panelView: root
                }
            }
        }
        */

        NetworkIndicator {
            id: networkIndicator
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                rightMargin: spacing
            }
            panelView: root
        }
        UserIndicator {
            id: userIndicator
            anchors {
                left: networkIndicator.right
                top: parent.top
                bottom: parent.bottom
                rightMargin: spacing
            }
            panelView: root
        }
        NotificationsIndicator {
            id: notificationsIndicator
            anchors {
                left: userIndicator.right
                top: parent.top
                bottom: parent.bottom
                rightMargin: spacing
            }
            panelView: root
        }
    }
}
