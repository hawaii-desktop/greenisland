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
    shellSurface: ShellSurface {
        onSetDefaultToplevel: {
            var pos = window.parent.randomPosition();
            window.parent.x = pos.x;
            window.parent.y = pos.y;

            if (d.windowType != WaylandWindowItem.TopLevelWindowType) {
                d.windowType = WaylandWindowItem.TopLevelWindowType;
                topLevelMapped();
                window.parent.raise();
            }

            window.maximizedChanged(false);
            window.fullScreenChanged(false);
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
        onSetPopup: {
            var parentWindow = windowManager.windowForSurface(parentSurface);
            if (parentWindow) {
                window.parent.parent = parentWindow;
                window.parent.x = relativeToParent.x;
                window.parent.y = relativeToParent.y;
            }

            d.windowType = WaylandWindowItem.PopupWindowType;
            window.popupMapped();
        }
        onSetMaximized: {
            // TODO: We should really move the window to output origin,
            // but that's tricky in a multi-output setup
            window.parent.x = 0;
            window.parent.y = 0;
            window.maximizedChanged(true);
        }
        onSetFullScreen: {
            // TODO: We should really move the window to output origin,
            // but that's tricky in a multi-output setup
            window.parent.x = 0;
            window.parent.y = 0;
            window.fullScreenChanged(true);
        }
        onTitleChanged: {
            window.titleChanged(title);
        }
        onClassNameChanged: {
            window.appIdChanged(className);
        }
    }
    onFocusChanged: {
        if (window.focus)
            window.parent.raise();
        window.activeChanged(window.focus);
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
        case WaylandWindowItem.PopupWindowType:
            popupDestroyed();
            break;
        }
    }

    QtObject {
        id: d

        property bool unresponsive: false
        property int windowType: WaylandWindowItem.UnknownWindowType
    }

    Timer {
        id: pingTimer
        interval: 250
        onTriggered: {
            console.warn("ShellSurface is unresponsive");
            d.unresponsive = true;
        }
    }
}

