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

XdgSurfaceItem {
    id: window
    transform: [
        Scale {
            id: scaleTransform
            origin.x: window.width / 2
            origin.y: window.height / 2
        }
    ]
    shellSurface: XdgSurface {
        onSetDefaultToplevel: {
            d.transientWindow = false;
            window.raise();
            mapAnimation.start();
        }
        onSetTransient: {
            var parentView = null;

            compositor.windows.forEach(function(element, index, array) {
                console.log(",,,,", element, element.surface, parentSurface);
                if (element.surface === parentSurface) {
                    parentView = element;
                    return;
                }
            });

            if (parentView) {
                window.x = (parentView.width - window.width) / 2;
                window.y = (parentView.height - window.height) / 2;
            } else {
                window.x = 0;
                window.y = 0;
            }

            d.transientWindow = true;
            transientMapAnimation.start();
        }
        onActiveChanged: {
            if (active)
                window.raise();
        }
        onMaximizedChanged: {
            if (maximized) {
                console.log("----------- MAXIMIZED", window.x, window.y);
                d.savedPosition = Qt.point(window.x, window.y);
                window.x = 0;
                window.y = 0;
            } else {
                console.log("--------------- NOT MAX", d.savedPosition.x, d.savedPosition.y);
                window.x = d.savedPosition.x;
                window.y = d.savedPosition.y;
            }
        }
        onPingRequested: {
            pingTimer.start();
        }
        onPong: {
            pingTimer.stop();
            d.unresponsive = false;
        }
        onWindowMenuRequested: {
            console.log("Window menu requested at " + position.x + "," + position.y);
        }
    }
    onSurfaceDestroyed: {
        // Destroy with an animation
        view.bufferLock = true;
        if (d.transientWindow)
            transientDestroyAnimation.start();
        else
            destroyAnimation.start();
    }

    QtObject {
        id: d

        property bool unresponsive: false
        property point savedPosition
        property bool transientWindow: false
    }

    Timer {
        id: pingTimer
        interval: 250
        onTriggered: {
            console.warn("XdgSurface is unresponsive");
            d.unresponsive = true;
        }
    }

    SequentialAnimation {
        id: mapAnimation

        ParallelAnimation {
            NumberAnimation {
                target: window
                property: "opacity"
                easing.type: Easing.OutExpo
                from: 0.0
                to: 1.0
                duration: 350
            }
            NumberAnimation {
                target: scaleTransform
                property: "xScale"
                easing.type: Easing.OutExpo
                from: 0.01
                to: 1.0
                duration: 350
            }
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                easing.type: Easing.OutExpo
                from: 0.1
                to: 1.0
                duration: 350
            }
        }
    }

    SequentialAnimation {
        id: destroyAnimation

        ParallelAnimation {
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                to: 2 / height
                duration: 150
            }
            NumberAnimation {
                target: scaleTransform
                property: "xScale"
                to: 0.4
                duration: 150
            }
        }
        NumberAnimation {
            target: scaleTransform
            property: "xScale"
            to: 0
            duration: 150
        }
        ScriptAction {
            script: {
                window.destroy();
            }
        }
    }

    ParallelAnimation {
        id: transientMapAnimation

        NumberAnimation {
            target: window
            property: "opacity"
            easing.type: Easing.OutQuad
            from: 0.0
            to: 1.0
            duration: 250
        }
        NumberAnimation {
            target: scaleTransform
            property: "yScale"
            easing.type: Easing.OutQuad
            from: 0.0
            to: 1.0
            duration: 250
        }
    }

    SequentialAnimation {
        id: transientDestroyAnimation

        ParallelAnimation {
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                easing.type: Easing.OutQuad
                from: 1.0
                to: 0.0
                duration: 200
            }
            NumberAnimation {
                target: window
                property: "opacity"
                easing.type: Easing.OutQuad
                to: 0.0
                duration: 200
            }
        }
        ScriptAction {
            script: {
                window.destroy();
            }
        }
    }
}
