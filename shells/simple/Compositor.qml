/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

import QtQuick 2.5
import GreenIsland 1.0

WaylandCompositor {
    property QtObject primarySurfacesArea: null

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
            windowManager.recalculateVirtualGeometry();
            console.timeEnd("output" + d.outputs.length - 1);
        }
        onScreenRemoved: {
            var index = screenManager.indexOf(screen);
            console.time("output" + index);
            if (index < d.outputs.length) {
                var output = d.outputs[index];
                d.outputs.splice(index, 1);
                output.destroy();
                windowManager.recalculateVirtualGeometry();
            }
            console.timeEnd("output" + index);
        }
        onPrimaryScreenChanged: {
            var index = screenManager.indexOf(screen);
            if (index < d.outputs.length) {
                compositor.primarySurfacesArea = d.outputs[index].surfacesArea;
                compositor.defaultOutput = d.outputs[index];
            }
        }
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+Backspace"
        onActivated: Qt.quit()
    }

    WindowManager {
        id: windowManager
        compositor: compositor
        onWindowCreated: {
            var i, output, view;
            for (i = 0; i < d.outputs.length; i++) {
                output = d.outputs[i];
                view = windowComponent.createObject(output.surfacesArea, {"window": window});
                view.initialize(window, output);
            }
        }

        Component.onCompleted: {
            initialize();
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

    Component {
        id: windowComponent

        WaylandWindow {}
    }
}
