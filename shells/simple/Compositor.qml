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
import GreenIsland 1.0 as GreenIsland

GreenIsland.WaylandCompositor {
    property QtObject primarySurfacesArea: null

    id: compositor
    onSurfaceRequested: {
        var surface = surfaceComponent.createObject(compositor, {});
        surface.initialize(compositor, client, id, version);
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+Backspace"
        onActivated: Qt.quit()
    }

    GreenIsland.ScreenManager {
        id: screenManager

        onScreenAdded: {
            var view = outputComponent.createObject(compositor, {
                "compositor": compositor,
                "nativeScreen": screen
            })

            __private.outputs.push(view)
        }
        onScreenRemoved: {
            var index = screenManager.indexOf(screen)

            if (index < __private.outputs.length) {
                var output = __private.outputs[index]
                __private.outputs.splice(index, 1)
                output.destroy()
            }
        }
        onPrimaryScreenChanged: {
            var index = screenManager.indexOf(screen)

            if (index < __private.outputs.length)
                compositor.defaultOutput = __private.outputs[index]
        }
    }

    GreenIsland.ApplicationManager {
        id: applicationManager
    }

    GreenIsland.WlShell {
        onWlShellSurfaceCreated: {
            var window = applicationManager.createWindow(shellSurface.surface)

            for (var i = 0; i < __private.outputs.length; i++) {
                var view = chromeComponent.createObject(__private.outputs[i].surfacesArea, {
                    "shellSurface": shellSurface, "window": window, "decorated": true
                })

                view.moveItem = window.moveItem
                window.addWindowView(view)
            }
        }
    }

    GreenIsland.XdgShell {
        property variant viewsBySurface: ({})

        onXdgSurfaceCreated: {
            var window = applicationManager.createWindow(xdgSurface.surface)

            var i, view;
            for (i = 0; i < __private.outputs.length; i++) {
                view = chromeComponent.createObject(__private.outputs[i].surfacesArea, {
                    "shellSurface": xdgSurface, "window": window, "decorated": false
                })

                view.moveItem = window.moveItem

                if (viewsBySurface[xdgSurface.surface] == undefined)
                    viewsBySurface[xdgSurface.surface] = new Array()

                viewsBySurface[xdgSurface.surface].push({
                    "output": __private.outputs[i], "view": view
                })
                window.addWindowView(view)
            }
        }
        onXdgPopupCreated: {
            var window = applicationManager.createWindow(xdgPopup.surface);

            var i, j, parentView, view, parentViews = viewsBySurface[xdgPopup.parentSurface];
            for (i = 0; i < __private.outputs.length; i++) {
                for (j = 0; j < parentViews.length; j++) {
                    if (parentViews[j].output == __private.outputs[i]) {
                        view = chromeComponent.createObject(parentViews[j].view, {"shellSurface": xdgPopup, "window": window});
                        view.x = xdgPopup.position.x;
                        view.y = xdgPopup.position.y;
                        view.moveItem = window.moveItem;
                        window.addWindowView(view);
                    }
                }
            }
        }
    }

    QtObject {
        id: __private

        property variant outputs: []
    }

    Component {
        id: outputComponent

        ScreenView {}
    }

    Component {
        id: surfaceComponent

        GreenIsland.WaylandSurface {}
    }

    Component {
        id: chromeComponent

        GreenIsland.WindowChrome {}
    }
}
