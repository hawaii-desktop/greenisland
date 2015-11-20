/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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
import QtQuick.Window 2.2
import GreenIsland 1.0

WaylandOutput {
    property QtObject screen
    property alias surfacesArea: backgroundLayer

    id: output
    manufacturer: screen.manufacturer
    model: screen.model
    position: screen.position
    physicalSize: screen.physicalSize
    subpixel: screen.subpixel
    transform: screen.transform
    scaleFactor: screen.scaleFactor
    sizeFollowsWindow: true
    window: Window {
        property QtObject output

        id: window
        x: screen.position.x
        y: screen.position.y
        width: screen.size.width
        height: screen.size.height
        flags: Qt.FramelessWindowHint
        visible: true

        LocalPointerTracker {
            id: localPointerTracker
            anchors.fill: parent
            globalTracker: globalPointerTracker

            Item {
                property alias zoomEnabled: zoomArea.enabled

                anchors.fill: parent
                transform: Scale {
                    id: screenScaler
                    origin.x: zoomArea.x2
                    origin.y: zoomArea.y2
                    xScale: zoomArea.zoom2
                    yScale: zoomArea.zoom2
                }

                ScreenZoom {
                    id: zoomArea
                    anchors.fill: parent
                    scaler: screenScaler
                    enabled: true

                    Text {
                        anchors {
                            top: parent.top
                            right: parent.right
                        }
                        z: 1000
                        text: fpsCounter.fps
                        font.pointSize: 36
                        style: Text.Raised
                        styleColor: "#222"
                        color: "white"
                        visible: false

                        FpsCounter {
                            id: fpsCounter
                        }
                    }

                    Image {
                        id: backgroundLayer
                        anchors.fill: parent
                        source: "images/wallpaper.png"
                        sourceSize.width: width
                        sourceSize.height: height
                        fillMode: Image.Tile
                    }
                }
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
