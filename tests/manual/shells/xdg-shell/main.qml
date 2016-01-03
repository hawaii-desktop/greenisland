/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

WaylandCompositor {
    property variant windows: []

    id: compositor
    extensions: [
        XdgShell {
            id: defaultShell
            onCreateSurface: {
                var item = windowComponent.createObject(screen.surfacesArea, {"surface": surface});
                item.shellSurface.initialize(defaultShell, surface, client, id);
                windows.push(item);

                // Remove from the windows list
                item.surfaceDestroyed.connect(function() {
                    var index = compositor.windows.indexOf(item);
                    if (index >= 0)
                        compositor.windows.splice(index, 1);
                });
            }
            onCreatePopup: {
                var i, item, parentItem = null;
                for (i = 0; i < windows.length; i++) {
                    item = windows[i];
                    if (item.surface === parentSurface) {
                        parentItem = item;
                        break;
                    }
                }

                if (!parentItem) {
                    console.error("Couldn't find a parent surface for the popup");
                    return;
                }

                item = popupComponent.createObject(parentItem, {
                                                       "x": relativeToParent.x,
                                                       "y": relativeToParent.y,
                                                       "surface": surface
                                                   });
                item.shellSurface.initialize(defaultShell, inputDevice, surface, client, id);
            }

            Component.onCompleted: {
                initialize();
            }
        },
        GtkShell {
            id: gtkShell
            onCreateSurface: {
                var gtkSurface = gtkSurfaceComponent.createObject(null, {"surface": surface});
                gtkSurface.appIdChanged.connect(function(appId) {
                    console.log("Surface", surface, "associated to", appId);
                });
                gtkSurface.initialize(gtkShell, surface, client, id);
            }

            Component.onCompleted: {
                initialize();
            }
        }
    ]
    onCreateSurface: {
        var surface = surfaceComponent.createObject(compositor, {});
        surface.initialize(compositor, client, id, version);
    }

    WaylandScreen {
        id: screen
        compositor: compositor

        Component.onCompleted: {
            compositor.defaultOutput = screen;
        }
    }

    Component {
        id: surfaceComponent

        WaylandSurface {}
    }

    Component {
        id: windowComponent

        XdgSurfaceChrome {}
    }

    Component {
        id: popupComponent

        XdgPopupChrome {}
    }

    Component {
        id: gtkSurfaceComponent

        GtkSurface {}
    }
}
