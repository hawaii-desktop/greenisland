/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

import QtQuick 2.0
import GreenIsland 1.0

ShellSurfaceItem {
    property bool animationsEnabled: true
    readonly property alias type: d.type
    readonly property alias appId: d.appId
    readonly property alias title: d.title
    readonly property alias windowGeometry: d.windowGeometry
    readonly property alias taskIconGeometry: d.taskIconGeometry
    readonly property alias minimized: d.minimized
    readonly property alias maximized: d.maximized
    readonly property alias fullscreen: d.fullscreen
    readonly property alias parentSurface: d.parentSurface

    id: windowChrome
    transform: [
        Scale {
            id: scaleTransform
            origin.x: windowChrome.width / 2
            origin.y: windowChrome.height / 2
        },
        Scale {
            id: scaleTransformPos
            origin.x: windowChrome.width / 2
            origin.y: windowChrome.y - windowChrome.height
        }
    ]
    opacity: 0.0
    onTypeChanged: {
        switch (d.type) {
        case 1:
            topLevelMapAnimation.start();
            break;
        case 2:
            transientMapAnimation.start();
            break;
        case 3:
            popupMapAnimation.start();
            break;
        }
    }
    onSurfaceDestroyed: {
        view.bufferLock = true;

        switch (windowChrome.type) {
        case 1:
            topLevelDestroyAnimation.start();
            break;
        case 2:
            transientDestroyAnimation.start();
            break;
        case 3:
            popupDestroyAnimation.start();
            break;
        default:
            windowChrome.destroy();
            break;
        }
    }

    QtObject {
        // 0: Unknown
        // 1: Top level
        // 2: Transient
        // 3: Popup
        property int type: 0
        property real x
        property real y
        property bool unresponsive: false
        property bool started: false
        property string title
        property string appId
        property rect windowGeometry: Qt.rect(0, 0, view.surface.size.width, view.surface.size.height)
        property rect taskIconGeometry
        property bool minimized: false
        property bool maximized: false
        property bool fullscreen: false
        property WaylandSurface parentSurface: null

        id: d
        onMinimizedChanged: {
            if (minimized)
                minimizeAnimation.start();
            else
                unminimizeAnimation.start();
        }
    }

    Timer {
        id: pingTimer
        interval: 250
        onTriggered: {
            console.warn("WindowChrome is unresponsive");
            d.unresponsive = true;
        }
    }

    Connections {
        target: shellSurface
        ignoreUnknownSignals: true

        // wl-shell
        onClassNameChanged: d.appId = shellSurface.className
        onSetDefaultToplevel: {
            d.type = 1;
            d.minimized = false;
            d.maximized = false;
            d.fullscreen = false;
            d.parentSurface = null;
        }
        onSetTransient: {
            d.type = 2;
            d.parentSurface = parentSurface;
        }
        onSetPopup: {
            d.type = 3;
            d.parentSurface = null;
        }
        onSetMaximized: {
            if (output)
                d.maximized = true;
        }
        onSetFullScreen: {
            if (output)
                d.fullscreen = true;
        }

        // xdg-shell
        onAppIdChanged: d.appId = shellSurface.appId
        onWindowGeometryChanged: d.windowGeometry = shellSurface.windowGeometry
        onParentSurfaceChanged: {
            d.type = 2;
            d.minimized = false;
            d.parentSurface = shellSurface.parentSurface;
        }
        onActivatedChanged: {
            if (shellSurface.activated && d.started)
                focusAnimation.start();
            d.minimized = false;
            d.parentSurface = null;
        }
        onSetMinimized: d.minimized = true
        onMaximizedChanged: d.maximized = shellSurface.maximized
        onFullscreenChanged: d.fullscreen = shellSurface.fullscreen
        onShowWindowMenu: {
            console.log("Window menu requested at " + localSurfacePosition.x + "," + localSurfacePosition.y);
        }

        // All
        onTitleChanged: d.title = shellSurface.title

        onPingRequested: {
            pingTimer.start();
        }
        onPong: {
            pingTimer.stop();
            d.unresponsive = false;
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
     * Generic animations
     */

    SequentialAnimation {
        id: focusAnimation

        ParallelAnimation {
            NumberAnimation {
                target: scaleTransform
                property: "xScale"
                easing.type: Easing.OutQuad
                to: 1.02
                duration: 100
            }
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                easing.type: Easing.OutQuad
                to: 1.02
                duration: 100
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: scaleTransform
                property: "xScale"
                easing.type: Easing.InOutQuad
                to: 1.0
                duration: 100
            }
            NumberAnimation {
                target: scaleTransform
                property: "yScale"
                easing.type: Easing.InOutQuad
                to: 1.0
                duration: 100
            }
        }
    }

    /*
     * Top level view animations
     */

    SequentialAnimation {
        id: topLevelMapAnimation

        ParallelAnimation {
            NumberAnimation {
                target: windowChrome
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

        ScriptAction {
            script: {
                d.started = true;
            }
        }
    }

    SequentialAnimation {
        id: topLevelDestroyAnimation

        ScriptAction {
            script: {
                d.started = false;
            }
        }

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
            target: windowChrome
            property: "opacity"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 200
        }

        ScriptAction {
            script: {
                windowChrome.destroy();
            }
        }
    }

    /*
     * Transient view animations
     */

    ParallelAnimation {
        id: transientMapAnimation

        NumberAnimation {
            target: windowChrome
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

        ScriptAction {
            script: {
                d.started = true;
            }
        }
    }

    SequentialAnimation {
        id: transientDestroyAnimation

        ScriptAction {
            script: {
                d.started = false;
            }
        }

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
                target: windowChrome
                property: "opacity"
                easing.type: Easing.OutQuad
                from: 1.0
                to: 0.0
                duration: 200
            }
        }

        ScriptAction {
            script: {
                windowChrome.destroy();
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
                target: windowChrome
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

        ScriptAction {
            script: {
                d.started = true;
            }
        }
    }

    ParallelAnimation {
        id: popupDestroyAnimation

        ScriptAction {
            script: {
                d.started = false;
            }
        }

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
            target: windowChrome
            property: "opacity"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 150
        }

        ScriptAction {
            script: {
                windowChrome.destroy();
            }
        }
    }

    /*
     * Minimize animations
     */

    ParallelAnimation {
        id: minimizeAnimation

        NumberAnimation {
            target: windowChrome
            property: "x"
            easing.type: Easing.OutQuad
            to: d.taskIconGeometry.x - (view.output ? view.output.position.x : 0)
            duration: 350
        }
        NumberAnimation {
            target: windowChrome
            property: "y"
            easing.type: Easing.OutQuad
            to: d.taskIconGeometry.y - (view.output ? view.output.position.y : 0)
            duration: 350
        }
        NumberAnimation {
            target: windowChrome
            property: "scale"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 500
        }
        NumberAnimation {
            target: windowChrome
            property: "opacity"
            easing.type: Easing.Linear
            to: 0.0
            duration: 500
        }
    }

    ParallelAnimation {
        id: unminimizeAnimation

        NumberAnimation {
            target: windowChrome
            property: "x"
            easing.type: Easing.OutQuad
            to: windowChrome.savedProperties.x
            duration: 350
        }
        NumberAnimation {
            target: windowChrome
            property: "y"
            easing.type: Easing.OutQuad
            to: windowChrome.savedProperties.y
            duration: 350
        }
        NumberAnimation {
            target: windowChrome
            property: "scale"
            easing.type: Easing.OutQuad
            to: 1.0
            duration: 500
        }
        NumberAnimation {
            target: windowChrome
            property: "opacity"
            easing.type: Easing.Linear
            to: 1.0
            duration: 500
        }
    }

    Component.onCompleted: d.type = 1
}
