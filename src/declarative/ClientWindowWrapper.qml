/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

WindowWrapper {
    property bool unresponsive: false

    Timer {
        id: pingPongTimer
        interval: 200
        onTriggered: unresponsive = true
    }

    Connections {
        target: window.surface
        onPong: {
            // Surface replied with a pong this means it's responsive
            pingPoingTimer.running = false;
            unresponsive = false;
        }
    }

    function pingSurface() {
        // Ping logic applies only to windows actually painted
        if (!window.paintEnabled)
            return;

        // Ping the surface to see whether it's responsive, if a pong
        // doesn't arrive before the timeout is trigger we know the
        // surface is unresponsive and raise the flag
        window.surface.ping();
        pingPongTimer.running = true;
    }
}
