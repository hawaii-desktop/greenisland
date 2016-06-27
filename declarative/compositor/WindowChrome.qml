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
import QtGraphicalEffects 1.0

ClientWindowItem {
    property QtObject window
    property bool animationsEnabled: false
    property bool decorated: false

    signal showWindowMenu(point localSurfacePosition)

    id: windowChrome
    x: window.moveItem.x - view.output.geometry.x
    y: window.moveItem.y - view.output.geometry.y
    transform: [
        Scale {
            id: scaleTransform
            origin.x: windowChrome.width / 2
            origin.y: windowChrome.height / 2
        },
        Scale {
            id: scaleTransformPos
            origin.x: windowChrome.width / 2
            origin.y: window.y - view.output.geometry.y - windowChrome.height
        }
    ]
    opacity: 0.0
    onSurfaceDestroyed: {
        view.bufferLock = true;

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
        default:
            windowChrome.destroy();
            break;
        }
    }

    QtObject {
        property real x
        property real y
        property bool unresponsive: false
        property bool started: false
        property bool animationsOverride: false

        id: d

        function showAnimation() {
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
    }

    QtObject {
        property real x
        property real y

        id: savedProperties
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
        target: window
        onTypeChanged: d.showAnimation()
        onActivatedChanged: {
            if (window.activated && d.started)
                focusAnimation.start();
        }
        onMinimizedChanged: {
            if (window.minimized)
                minimizeAnimation.start();
            else
                unminimizeAnimation.start();
        }
        onShowWindowMenu: windowChrome.showWindowMenu(localSurfacePosition)
        /*
        onPingRequested: {
            pingTimer.start();
        }
        onPong: {
            pingTimer.stop();
            d.unresponsive = false;
        }
        */
    }

    // Shadow
    // FIXME: Transparent backgrounds will be opaque due to shadows
    RectangularGlow {
        id: dropShadow
        anchors.fill: parent
        glowRadius: 10
        spread: 0.2
        color: "#80000000"
        z: -1
        visible: decorated && !window.maximized
    }

    /*
     * Behavior
     */

    Behavior on x {
        enabled: animationsEnabled || d.animationsOverride
        SmoothedAnimation {
            easing.type: Easing.InOutQuad
            duration: 450
        }
    }

    Behavior on y {
        enabled: animationsEnabled || d.animationsOverride
        SmoothedAnimation {
            easing.type: Easing.InOutQuad
            duration: 450
        }
    }

    Behavior on width {
        enabled: animationsEnabled || d.animationsOverride
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    Behavior on height {
        enabled: animationsEnabled || d.animationsOverride
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 350
        }
    }

    Behavior on scale {
        enabled: animationsEnabled || d.animationsOverride
        SmoothedAnimation {
            easing.type: Easing.OutQuad
            duration: 450
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
                to: 2 / windowChrome.height
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

    SequentialAnimation {
        id: minimizeAnimation

        ScriptAction {
            script: {
                savedProperties.x = window.moveItem.x;
                savedProperties.y = window.moveItem.y;
                d.animationsOverride = true;
                window.moveItem.x = window.taskIconGeometry.x;
                window.moveItem.y = window.taskIconGeometry.y;
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: windowChrome
                property: "scale"
                easing.type: Easing.OutQuad
                to: 0.0
                duration: 550
            }
            NumberAnimation {
                target: windowChrome
                property: "opacity"
                easing.type: Easing.Linear
                to: 0.0
                duration: 500
            }
        }

        ScriptAction {
            script: {
                d.animationsOverride = false;
            }
        }
    }

    SequentialAnimation {
        id: unminimizeAnimation

        ScriptAction {
            script: {
                d.animationsOverride = true;
                window.moveItem.x = savedProperties.x;
                window.moveItem.y = savedProperties.y;
                d.animationsOverride = false;
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: windowChrome
                property: "scale"
                easing.type: Easing.OutQuad
                to: 1.0
                duration: 550
            }
            NumberAnimation {
                target: windowChrome
                property: "opacity"
                easing.type: Easing.Linear
                to: 1.0
                duration: 500
            }
        }
    }
}
