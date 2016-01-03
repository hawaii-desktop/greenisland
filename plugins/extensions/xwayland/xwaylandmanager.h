/****************************************************************************
 * This file is part of Hawaii.
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

#ifndef XWAYLANDMANAGER_H
#define XWAYLANDMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>

#include <xcb/xcb.h>

namespace Xcb {
class Window;
class Resources;
}

namespace GreenIsland {

class Compositor;
class XWayland;
class XWaylandWindow;
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

    XWaylandManager(Compositor *compositor, XWaylandServer *server,
                    QObject *parent = 0);
    ~XWaylandManager();

    void start(int fd);

    void addWindow(xcb_window_t id, XWaylandWindow *window);
    void removeWindow(xcb_window_t id);

    XWaylandWindow *windowFromId(xcb_window_t id);

private:
    Compositor *m_compositor;
    XWaylandServer *m_server;
    xcb_visualid_t m_visualId;
    xcb_colormap_t m_colorMap;

    xcb_selection_request_event_t m_selRequest;
    xcb_window_t m_selWindow;

    xcb_cursor_t *m_cursors;
    int m_lastCursor;

    Xcb::Window *m_wmWindow;

    QMap<xcb_window_t, XWaylandWindow *> m_windowsMap;
    QList<XWaylandWindow *> m_unpairedWindows;
    XWaylandWindow *m_focusWindow;

    void setupVisualAndColormap();
    void wmSelection();
    void initializeDragAndDrop();

    void setCursor(xcb_window_t window, const CursorType &cursor);

    void createWindowManager();
    void createFrame(XWaylandWindow *window);

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

    void handleMoveResize(XWaylandWindow *window,
                          xcb_client_message_event_t *event);
    void handleState(XWaylandWindow *window,
                     xcb_client_message_event_t *event);
    void handleSurfaceId(XWaylandWindow *window,
                         xcb_client_message_event_t *event);

    friend class XWayland;

private Q_SLOTS:
    void wmEvents();
};

}

#endif // XWAYLANDMANAGER_H
