/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

import QtQuick 2.0
import QtQuick.Window 2.2
import GreenIsland 1.0

WaylandOutput {
    property alias surfacesArea: backgroundLayer

    id: output
    manufacturer: nativeScreen.manufacturer
    model: nativeScreen.model
    position: nativeScreen.position
    physicalSize: nativeScreen.physicalSize
    subpixel: nativeScreen.subpixel
    transform: nativeScreen.transform
    scaleFactor: nativeScreen.scaleFactor
    sizeFollowsWindow: true
    window: Window {
        property QtObject output

        id: window
        x: nativeScreen.position.x
        y: nativeScreen.position.y
        width: nativeScreen.size.width
        height: nativeScreen.size.height
        flags: Qt.FramelessWindowHint
        visible: false

        KeyBindingsFilter {
            anchors.fill: parent
            keyBindings: compositor.keyBindingsManager
        }

        LocalPointerTracker {
            id: localPointerTracker
            anchors.fill: parent
            globalTracker: globalPointerTracker

            Image {
                id: backgroundLayer
                anchors.fill: parent
                source: "images/wallpaper.png"
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.Tile
            }

            PointerItem {
                id: cursor
                inputDevice: output.compositor.defaultInputDevice
                x: localPointerTracker.mouseX - hotspotX
                y: localPointerTracker.mouseY - hotspotY
                visible: globalPointerTracker.output === output
            }
        }
    }
}
