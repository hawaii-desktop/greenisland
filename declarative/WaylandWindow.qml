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

ShellSurfaceItem {
    property variant windows: []

    id: window
    transform: [
        Scale {
            id: scaleTransform
            origin.x: window.width / 2
            origin.y: window.height / 2
        }
    ]
    shellSurface: ShellSurface {
        onSetDefaultToplevel: {
            window.raise();
            mapAnimation.start();
        }
        onSetTransient: {
            var parentView = null;

            windows.forEach(function(element, index, array) {
                if (element.surface === parentSurface) {
                    parentView = element;
                    return;
                }
            });

            if (parentView) {
                window.x = parentView.x + relativeToParent.x;
                window.y = parentView.y + relativeToParent.y;
            }
        }
        onSetPopup: {
            var parentView = null;

            windows.forEach(function(element, index, array) {
                if (element.surface === parent) {
                    parentView = element;
                    return;
                }
            });

            if (parentView) {
                window.parent = parentView;
                window.x = relativeToParent.x;
                window.y = relativeToParent.y;
            }
        }
    }
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
}

