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

LauncherDropItem {
    id: launcher

    // Icon size
    // TODO: From settings
    property real iconSize: 48

    // Tile size
    property real tileSize: iconSize + 12

    // Orientation
    property alias orientation: view.orientation

    // Number of items
    property alias count: view.count

    onApplicationDropped: visualModel.model.pinApplication(path)
    onUrlDropped: visualModel.model.pinUrl(url)

    LauncherAppChooserModel {
        id: appchooserModel
    }

    LauncherModel {
        id: launcherModel
        objectName: "launcherModel"
    }

    VisualDataModel {
        id: visualModel

        model: ListAggregatorModel {
            id: items
        }
        delegate: LauncherViewItem {}
    }

    ListView {
        id: view
        anchors.fill: parent

        focus: true
        model: visualModel
        cacheBuffer: 10000
        interactive: false

        add: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 400 }
            NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 400 }
        }

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
        }
    }

    Component.onCompleted: {
        items.appendModel(appchooserModel);
        items.appendModel(launcherModel);
    }
}
