/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef XWAYLANDMANAGER_H
#define XWAYLANDMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>

#include <xcb/xcb.h>

namespace Xcb {
class Window;
class Resources;
}

class XWayland;
class XWaylandShellSurface;
class XWaylandServer;

class XWaylandManager : public QObject
{
    Q_OBJECT
public:
    enum CursorType {
        CursorUnset = -1,
        CursorTop,
        CursorBottom,
        CursorLeft,
        CursorRight,
        CursorTopLeft,
        CursorTopRight,
        CursorBottomLeft,
        CursorBottomRight,
        CursorLeftPointer
    };

    XWaylandManager(XWaylandServer *server, QObject *parent = Q_NULLPTR);
    ~XWaylandManager();

    void start(int fd);

    void addWindow(xcb_window_t id, XWaylandShellSurface *shellSurface);
    void removeWindow(xcb_window_t id);

    XWaylandShellSurface *shellSurfaceFromId(xcb_window_t id);

Q_SIGNALS:
    void shellSurfaceAdded(XWaylandShellSurface *shellSurface);
    void shellSurfaceRemoved(XWaylandShellSurface *shellSurface);

private:
    XWaylandServer *m_server;
    xcb_visualid_t m_visualId;
    xcb_colormap_t m_colorMap;

    xcb_selection_request_event_t m_selRequest;
    xcb_window_t m_selWindow;

    xcb_cursor_t *m_cursors;
    int m_lastCursor;

    Xcb::Window *m_wmWindow;

    QMap<xcb_window_t, XWaylandShellSurface *> m_windowsMap;
    QList<XWaylandShellSurface *> m_unpairedWindows;
    XWaylandShellSurface *m_focusWindow;

    void setupVisualAndColormap();
    void wmSelection();
    void initializeDragAndDrop();

    void setCursor(xcb_window_t window, const CursorType &cursor);

    void createWindowManager();

    void handleButton(xcb_button_press_event_t *event);
    void handleEnter(xcb_enter_notify_event_t *event);
    void handleLeave(xcb_leave_notify_event_t *event);
    void handleMotion(xcb_motion_notify_event_t *event);
    void handleCreateNotify(xcb_create_notify_event_t *event);
    void handleMapRequest(xcb_map_request_event_t *event);
    void handleMapNotify(xcb_map_notify_event_t *event);
    void handleUnmapNotify(xcb_unmap_notify_event_t *event);
    void handleReparentNotify(xcb_reparent_notify_event_t *event);
    void handleConfigureRequest(xcb_configure_request_event_t *event);
    void handleConfigureNotify(xcb_configure_notify_event_t *event);
    void handleDestroyNotify(xcb_destroy_notify_event_t *event);
    void handlePropertyNotify(xcb_property_notify_event_t *event);
    void handleClientMessage(xcb_client_message_event_t *event);

    bool handleSelection(xcb_generic_event_t *event);
    void handleSelectionNotify(xcb_selection_notify_event_t *event);

    void handleMoveResize(XWaylandShellSurface *window,
                          xcb_client_message_event_t *event);
    void handleState(XWaylandShellSurface *window,
                     xcb_client_message_event_t *event);
    void handleSurfaceId(XWaylandShellSurface *window,
                         xcb_client_message_event_t *event);

    friend class XWayland;

private Q_SLOTS:
    void wmEvents();
};

#endif // XWAYLANDMANAGER_H
