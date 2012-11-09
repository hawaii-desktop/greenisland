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
import FluidUi 1.0

Item {
    id: root

    signal itemClicked()

    width: children.legnth > 0 ? children[0].width : 50
    height: children.length > 0 ? children[0].height : 30

    onWidthChanged: console.log("content width", width)
    onHeightChanged: console.log("content height", height)
    onChildrenChanged: setupChildren()

    function setupChildren() {
        // Connect children to the itemClicked signal so that the menu
        // can handle a single item being clicked
        for (var i = 0; i < children.length; ++i) {
            if (children[i].clicked != undefined)
                children[i].clicked.connect(root.itemClicked);
        }
    }
}
