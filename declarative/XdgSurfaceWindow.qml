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

XdgSurfaceItem {
    signal titleChanged(string title)
    signal appIdChanged(string appId)

    signal activeChanged(bool active)
    signal minimizedChanged(bool minimized)
    signal maximizedChanged(bool maximized)
    signal fullScreenChanged(bool fullScreen)

    signal topLevelMapped()
    signal topLevelDestroyed()

    signal popupMapped()
    signal popupDestroyed()

    signal transientMapped()
    signal transientDestroyed()

    id: window
    shellSurface: XdgSurface {
        onSetDefaultToplevel: {
            var pos = window.parent.randomPosition();
            window.parent.x = pos.x;
            window.parent.y = pos.y;

            if (d.windowType != WaylandWindowItem.TopLevelWindowType) {
                d.windowType = WaylandWindowItem.TopLevelWindowType;
                topLevelMapped();
                window.parent.raise();
            }
        }
        onSetTransient: {
            var parentWindow = windowManager.windowForSurface(parentSurface);
            if (parentWindow) {
                window.parent.x = parentWindow.x + relativeToParent.x;
                window.parent.y = parentWindow.y + relativeToParent.y;
            } else {
                window.parent.x = (window.parent.parent.width - window.width) / 2;
                window.parent.y = (window.parent.parent.height - window.height) / 2;
            }

            d.windowType = WaylandWindowItem.TransientWindowType;
            transientMapped();
        }
        onActiveChanged: {
            if (active)
                window.parent.raise();
            window.activeChanged(active)
        }
        onMinimize: {
            window.minimizedChanged(true)
        }
        onMaximizedChanged: {
            if (maximized) {
                d.savedPosition = Qt.point(window.x, window.y);
                window.parent.x = 0;
                window.parent.y = 0;
            } else {
                window.parent.x = d.savedPosition.x;
                window.parent.y = d.savedPosition.y;
            }
            window.maximizedChanged(maximized)
        }
        onTitleChanged: {
            window.titleChanged(title);
        }
        onAppIdChanged: {
            window.appIdChanged(appId);
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
        switch (d.windowType) {
        case WaylandWindowItem.TopLevelWindowType:
            topLevelDestroyed();
            break;
        case WaylandWindowItem.TransientWindowType:
            transientDestroyed();
            break;
        }
    }

    QtObject {
        id: d

        property bool unresponsive: false
        property point savedPosition
        property int windowType: WaylandWindowItem.UnknownWindowType
    }

    Timer {
        id: pingTimer
        interval: 250
        onTriggered: {
            console.warn("XdgSurface is unresponsive");
            d.unresponsive = true;
        }
    }
}
