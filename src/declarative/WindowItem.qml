/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
import QtGraphicalEffects 1.0
import GreenIsland 1.0

SurfaceItem {
    id: surfaceItem
    clientRenderingEnabled: true
    touchEventsEnabled: true
    opacity: 0.0
    transform: [
        Scale {
            id: createScaleTransform
            origin.x: surfaceItem.width / 2
            origin.y: surfaceItem.height / 2
            xScale: 0.01
            yScale: 0.1
        },
        Scale {
            id: destroyScaleTransform
            origin.x: surfaceItem.width / 2
            origin.y: surfaceItem.height / 2
            xScale: 1.0
            yScale: 1.0
        }
    ]
    states: [
        State {
            name: "focused"
            when: surfaceItem.focus && !surfaceItem.unresponsive
            PropertyChanges { target: surfaceItem; z: 1 }
            PropertyChanges { target: unresponsiveEffect; opacity: 0.0 }
        },
        State {
            name: "unfocused"
            when: !surfaceItem.focus && !surfaceItem.unresponsive
            PropertyChanges { target: surfaceItem; z: 0 }
            PropertyChanges { target: unresponsiveEffect; opacity: 0.0 }
        },
        State {
            name: "focused-unresponsive"
            when: surfaceItem.focus && surfaceItem.unresponsive
            PropertyChanges { target: surfaceItem; z: 1 }
            PropertyChanges { target: unresponsiveEffect; opacity: 1.0 }
        },
        State {
            name: "unfocused-unresponsive"
            when: !surfaceItem.focus && surfaceItem.unresponsive
            PropertyChanges { target: surfaceItem; z: 0 }
            PropertyChanges { target: unresponsiveEffect; opacity: 1.0 }
        }
    ]
    onSurfaceChanged: renderer.source = surfaceItem

    property bool animationsEnabled: true

    // Render the item taking care of y inverted surfaces
    SurfaceRenderer {
        id: renderer
        anchors.fill: surfaceItem
        opacity: 1.0
    }

    // Unresponsive effect
    Colorize {
        id: unresponsiveEffect
        anchors.fill: renderer
        source: renderer
        hue: 0.0
        saturation: 0.5
        lightness: -0.2
        opacity: 0.0

        Behavior on opacity {
            enabled: surfaceItem.animationsEnabled
            NumberAnimation { easing.type: Easing.Linear; duration: 250 }
        }
    }

    ParallelAnimation {
        id: createAnimation

        NumberAnimation {
            target: surfaceItem
            property: "opacity"
            easing.type: Easing.Linear
            to: 1.0
            duration: 250
        }

        NumberAnimation {
            target: createScaleTransform
            property: "xScale"
            easing.type: Easing.OutExpo
            to: 1.0
            duration: 350
        }

        NumberAnimation {
            target: createScaleTransform
            property: "yScale"
            easing.type: Easing.OutExpo
            to: 1.0
            duration: 350
        }
    }

    ParallelAnimation {
        id: destroyAnimation

        NumberAnimation {
            target: destroyScaleTransform
            property: "xScale"
            easing.type: Easing.Linear
            to: 0.0
            duration: 250
        }

        NumberAnimation {
            target: destroyScaleTransform
            property: "yScale"
            easing.type: Easing.Linear
            to: 0.0
            duration: 250
        }

        NumberAnimation {
            target: surfaceItem
            property: "opacity"
            easing.type: Easing.Linear
            to: 0.0
            duration: 300
        }
    }

    function runCreateAnimation() {
        if (surfaceItem.animationsEnabled)
            createAnimation.start();
    }

    function runDestroyAnimation() {
        if (surfaceItem.animationsEnabled)
            destroyAnimation.start();
    }
}
