/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

ClientWindowView {
    property QtObject window
    property bool animationsEnabled: true

    id: view
    transform: [
        Scale {
            id: scaleTransform
            origin.x: view.width / 2
            origin.y: view.height / 2
        },
        Scale {
            id: scaleTransformPos
            origin.x: view.width / 2
            origin.y: view.y - view.height
        }
    ]
    opacity: 0.0

    QtObject {
        id: d

        property real x
        property real y
        property bool unresponsive: false
    }

    Timer {
        id: pingTimer
        interval: 250
        onTriggered: {
            console.warn("WaylandWindow is unresponsive");
            d.unresponsive = true;
        }
    }

    Connections {
        target: window
        onTypeChanged: {
            switch (window.type) {
            case ClientWindow.TopLevel:
                topLevelMapAnimation.start();
                break;
            case ClientWindow.Transient:
                transientMapAnimation.start();
                break;
            case ClientWindow.Popup:
                popupMapAnimation.start();
                break;
            }
        }
        onPingRequested: {
            pingTimer.start();
        }
        onPong: {
            pingTimer.stop();
            d.unresponsive = false;
        }
        onMinimizedChanged: {
            if (window.minimized)
                minimizeAnimation.start();
            else
                unminimizeAnimation.start();
        }
        onWindowMenuRequested: {
            console.log("Window menu requested at " + position.x + "," + position.y);
        }
    }

    Connections {
        target: window ? window.surface : null
        onSurfaceDestroyed: {
            switch (window.type) {
            case ClientWindow.TopLevel:
                topLevelDestroyAnimation.start();
                break;
            case ClientWindow.Transient:
                transientDestroyAnimation.start();
                break;
            case ClientWindow.Popup:
                popupDestroyAnimation.start();
                break;
            }
        }
    }

    /*
     * Behavior
     */

    Behavior on x {
        enabled: animationsEnabled
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    Behavior on y {
        enabled: animationsEnabled
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    Behavior on width {
        enabled: animationsEnabled
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    Behavior on height {
        enabled: animationsEnabled
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    Behavior on scale {
        enabled: animationsEnabled
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    /*
     * Top level view animations
     */

    SequentialAnimation {
        id: topLevelMapAnimation

        ParallelAnimation {
            NumberAnimation {
                target: view
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
        id: topLevelDestroyAnimation

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
        NumberAnimation {
            target: view
            property: "opacity"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 200
        }
        ScriptAction {
            script: {
                view.destroy();
            }
        }
    }

    /*
     * Transient view animations
     */

    ParallelAnimation {
        id: transientMapAnimation

        NumberAnimation {
            target: view
            property: "opacity"
            easing.type: Easing.OutQuad
            from: 0.0
            to: 1.0
            duration: 250
        }
        NumberAnimation {
            target: scaleTransformPos
            property: "xScale"
            easing.type: Easing.OutExpo
            from: 0.0
            to: 1.0
            duration: 250
        }
        NumberAnimation {
            target: scaleTransformPos
            property: "yScale"
            easing.type: Easing.OutExpo
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
                property: "xScale"
                easing.type: Easing.OutQuad
                from: 1.0
                to: 0.0
                duration: 200
            }
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                easing.type: Easing.OutQuad
                from: 1.0
                to: 0.0
                duration: 200
            }
            NumberAnimation {
                target: view
                property: "opacity"
                easing.type: Easing.OutQuad
                from: 1.0
                to: 0.0
                duration: 200
            }
        }
        ScriptAction {
            script: {
                view.destroy();
            }
        }
    }

    /*
     * Popup view animations
     */

    SequentialAnimation {
        id: popupMapAnimation

        ParallelAnimation {
            NumberAnimation {
                target: view
                property: "opacity"
                easing.type: Easing.OutQuad
                from: 0.0
                to: 1.0
                duration: 150
            }
            NumberAnimation {
                target: scaleTransform
                property: "xScale"
                easing.type: Easing.OutExpo
                from: 0.9
                to: 1.0
                duration: 150
            }
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                easing.type: Easing.OutExpo
                from: 0.9
                to: 1.0
                duration: 150
            }
        }
    }

    ParallelAnimation {
        id: popupDestroyAnimation

        NumberAnimation {
            target: scaleTransform
            property: "xScale"
            easing.type: Easing.OutExpo
            from: 1.0
            to: 0.8
            duration: 150
        }
        NumberAnimation {
            target: scaleTransform
            property: "yScale"
            easing.type: Easing.OutExpo
            from: 1.0
            to: 0.8
            duration: 150
        }
        NumberAnimation {
            target: view
            property: "opacity"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 150
        }
        ScriptAction {
            script: {
                view.destroy();
            }
        }
    }

    /*
     * Minimize animations
     */

    ParallelAnimation {
        id: minimizeAnimation

        NumberAnimation {
            target: view
            property: "x"
            easing.type: Easing.OutQuad
            to: window.taskIconGeometry.x - (view.output ? view.output.position.x : 0)
            duration: 350
        }
        NumberAnimation {
            target: view
            property: "y"
            easing.type: Easing.OutQuad
            to: window.taskIconGeometry.y - (view.output ? view.output.position.y : 0)
            duration: 350
        }
        NumberAnimation {
            target: view
            property: "scale"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 500
        }
        NumberAnimation {
            target: view
            property: "opacity"
            easing.type: Easing.Linear
            to: 0.0
            duration: 500
        }
    }

    ParallelAnimation {
        id: unminimizeAnimation

        NumberAnimation {
            target: view
            property: "x"
            easing.type: Easing.OutQuad
            to: view.savedProperties.x
            duration: 350
        }
        NumberAnimation {
            target: view
            property: "y"
            easing.type: Easing.OutQuad
            to: view.savedProperties.y
            duration: 350
        }
        NumberAnimation {
            target: view
            property: "scale"
            easing.type: Easing.OutQuad
            to: 1.0
            duration: 500
        }
        NumberAnimation {
            target: view
            property: "opacity"
            easing.type: Easing.Linear
            to: 1.0
            duration: 500
        }
    }
}
