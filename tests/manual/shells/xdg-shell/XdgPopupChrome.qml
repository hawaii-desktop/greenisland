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

XdgPopupItem {
    id: window
    transform: [
        Scale {
            id: scaleTransform
            origin.x: window.width / 2
            origin.y: window.height / 2
        }
    ]
    shellSurface: XdgPopup {}
    onFocusChanged: {
        if (window.focus)
            window.raise();
    }
    onSurfaceDestroyed: {
        view.bufferLock = true;
        destroyAnimation.start();
    }

    SequentialAnimation {
        id: mapAnimation

        ParallelAnimation {
            NumberAnimation {
                target: window
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
        id: destroyAnimation

        NumberAnimation {
            target: window
            property: "scale"
            easing.type: Easing.OutQuad
            to: 0.8
            duration: 150
        }
        NumberAnimation {
            target: window
            property: "opacity"
            easing.type: Easing.OutQuad
            to: 0.0
            duration: 150
        }
        ScriptAction {
            script: {
                window.destroy();
            }
        }
    }
}
