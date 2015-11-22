/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

WaylandCompositor {
    property QtObject primarySurfacesArea: null
    property variant additionalExtensions: []

    id: windowManager
    extensions: [
        Shell {
            id: wlShell
            onCreateShellSurface: {
                var item = wlWindowComponent.createObject(null, {"surface": surface});
                item.shellSurface.initialize(wlShell, surface, client, id);
                d.addWindow(d.createWindow(item));
            }

            Component.onCompleted: {
                initialize();
            }
        },
        XdgShell {
            id: xdgShell
            onCreateSurface: {
                var item = xdgWindowComponent.createObject(null, {"surface": surface});
                item.shellSurface.initialize(xdgShell, surface, client, id);
                d.addWindow(d.createWindow(item));
            }
            onCreatePopup: {
                var parentWindow = windowForSurface(parentSurface);
                if (!parentWindow) {
                    console.error("Couldn't find parent window for the popup");
                    return;
                }

                var item = xdgPopupComponent.createObject(null, {
                                                          "x": relativeToParent.x,
                                                          "y": relativeToParent.y,
                                                          "surface": surface
                                                      });
                item.shellSurface.initialize(xdgShell, inputDevice, surface, client, id);

                var window = d.createWindow(item);
                window.parent = parentWindow;
                d.addWindow(window);
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

    QtObject {
        id: d

        property variant windows: []

        function createWindow(item) {
            var window = windowComponent.createObject(primarySurfacesArea,
                                                      {"shellSurfaceItem": item});

            item.moveItem = window;

            item.topLevelMapped.connect(window.runTopLevelMapAnimation);
            item.topLevelDestroyed.connect(window.runTopLevelDestroyAnimation);
            item.popupMapped.connect(window.runPopupMapAnimation);
            item.popupDestroyed.connect(window.runPopupDestroyAnimation);
            item.transientMapped.connect(window.runTransientMapAnimation);
            item.transientDestroyed.connect(window.runTransientDestroyAnimation);
            item.surfaceDestroyed.connect(function() {
                var index = windows.indexOf(window);
                if (index >= 0)
                    windows.splice(index, 1);
            });

            return window;
        }

        function addWindow(window) {
            // Add to the windows list
            windows.push(window);
        }
    }

    Component {
        id: windowComponent

        WaylandWindow {}
    }

    Component {
        id: wlWindowComponent

        ShellSurfaceWindow {}
    }

    Component {
        id: xdgWindowComponent

        XdgSurfaceWindow {}
    }

    Component {
        id: xdgPopupComponent

        XdgPopupWindow {}
    }

    Component {
        id: gtkSurfaceComponent

        GtkSurface {}
    }

    Component.onCompleted: {
        //extensions += additionalExtensions
    }

    function windowForSurface(surface) {
        var window = null;
        d.windows.forEach(function(element, index, array) {
            if (element.shellSurfaceItem.surface === surface) {
                window = element;
                return;
            }
        });
        return window;
    }
}
