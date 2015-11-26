/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

import QtQuick 2.0
import GreenIsland 1.0

WindowManager {
    id: compositor
    onCreateSurface: {
        var surface = surfaceComponent.createObject(compositor, {});
        surface.initialize(compositor, client, id, version);
    }

    GlobalPointerTracker {
        id: globalPointerTracker
        compositor: compositor
    }

    ScreenManager {
        id: screenManager
        onScreenAdded: {
            console.time("output" + d.outputs.length);
            var view = screenComponent.createObject(
                        compositor, {
                            "compositor": compositor,
                            "nativeScreen": screen
                        });
            d.outputs.push(view);
            console.timeEnd("output" + d.outputs.length - 1);
        }
        onScreenRemoved: {
            var index = screenManager.indexOf(screen);
            if (index < d.outputs.length) {
                var output = d.outputs[index];
                d.outputs.splice(index, 1);
                output.destroy();
            }
        }
        onPrimaryScreenChanged: {
            var index = screenManager.indexOf(screen);
            if (index < d.outputs.length) {
                compositor.primarySurfacesArea = d.outputs[index].surfacesArea;
                compositor.defaultOutput = d.outputs[index];
            }
        }
    }

    QtObject {
        id: d

        property variant outputs: []
    }

    Component {
        id: screenComponent

        ScreenView {}
    }

    Component {
        id: surfaceComponent

        WaylandSurface {}
    }
}
