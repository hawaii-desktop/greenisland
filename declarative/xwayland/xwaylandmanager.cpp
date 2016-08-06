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

/*
 * This code takes inspiration from the xwayland module from Weston,
 * which is:
 *
 * Copyright (C) 2011 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <QtCore/QSocketNotifier>
#include <QtCore/QtMath>

#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>

#include <GreenIsland/Server/ClientWindow>

#include "xcbcursors.h"
#include "xcbwindow.h"
#include "xcbwrapper.h"
#include "xcbresources.h"
#include "xwayland.h"
#include "xwaylandmanager.h"
#include "xwaylandshellsurface.h"
#include "xwaylandserver.h"
//#include "xwaylandsurface.h"

#include <unistd.h>
#include <xcb/composite.h>
#include <wayland-server.h>

XWaylandManager::XWaylandManager(XWaylandServer *server, QObject *parent)
    : QObject(parent)
    , m_server(server)
    , m_cursors(Q_NULLPTR)
    , m_lastCursor(CursorUnset)
    , m_wmWindow(Q_NULLPTR)
    , m_focusWindow(Q_NULLPTR)
{
}

XWaylandManager::~XWaylandManager()
{
    Xcb::Cursors::destroyCursors(m_cursors);
    delete m_wmWindow;
    Xcb::closeConnection();
}

void XWaylandManager::start(int fd)
{
    // Connect to XCB (xcb_connect_to_fd() takes ownership of the fd)
    qCDebug(XWAYLAND) << "Connect to X11";
    Xcb::openConnection(fd);
    if (xcb_connection_has_error(Xcb::connection())) {
        qCWarning(XWAYLAND) << "Connection failed!";
        ::close(fd);
        return;
    }

    // Listen to WM events
    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(wmEvents()));

    // Resources and atoms
    Xcb::resources();

    // Visual and colormap
    setupVisualAndColormap();

    // Root window attributes
    quint32 values[1];
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
            XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(Xcb::connection(), Xcb::rootWindow(),
                                 XCB_CW_EVENT_MASK, values);

    // Redirect subwindows
    xcb_composite_redirect_subwindows(Xcb::connection(), Xcb::rootWindow(),
                                      XCB_COMPOSITE_REDIRECT_MANUAL);

    // Claim supported atoms
    quint32 supported[5];
    supported[0] = Xcb::resources()->atoms->net_wm_moveresize;
    supported[1] = Xcb::resources()->atoms->net_wm_state;
    supported[2] = Xcb::resources()->atoms->net_wm_state_fullscreen;
    supported[3] = Xcb::resources()->atoms->net_wm_state_maximized_vert;
    supported[4] = Xcb::resources()->atoms->net_wm_state_maximized_horz;
    xcb_change_property(
                Xcb::connection(), XCB_PROP_MODE_REPLACE,
                Xcb::rootWindow(),
                Xcb::resources()->atoms->net_supported,
                XCB_ATOM_ATOM, 32,
                (sizeof supported / sizeof supported[0]), supported
            );

    // Clain WM selection
    wmSelection();

    // Initialize DND
    initializeDragAndDrop();

    // Flush connection
    xcb_flush(Xcb::connection());

#if 0
    // Setup cursors
    m_cursors = Xcb::Cursors::createCursors();
    setCursor(Xcb::rootWindow(), CursorLeftPointer);
#endif

    // Create window and take WM_S0 selection, this will signal
    // Xwayland that the setup is done
    createWindowManager();

    qCDebug(XWAYLAND) << "X window manager created, root" << Xcb::rootWindow();
}

void XWaylandManager::addWindow(xcb_window_t id, XWaylandShellSurface *shellSurface)
{
    m_windowsMap[id] = shellSurface;
    Q_EMIT shellSurfaceAdded(shellSurface);
}

void XWaylandManager::removeWindow(xcb_window_t id)
{
    Q_EMIT shellSurfaceRemoved(m_windowsMap[id]);
    m_windowsMap.remove(id);
}

XWaylandShellSurface *XWaylandManager::shellSurfaceFromId(xcb_window_t id)
{
    return m_windowsMap.value(id, Q_NULLPTR);
}

void XWaylandManager::setupVisualAndColormap()
{
    xcb_depth_iterator_t depthIterator =
            xcb_screen_allowed_depths_iterator(Xcb::screen());
    xcb_visualtype_t *visualType = Q_NULLPTR;
    xcb_visualtype_iterator_t visualTypeIterator;
    while (depthIterator.rem > 0) {
        if (depthIterator.data->depth == 32) {
            visualTypeIterator = xcb_depth_visuals_iterator(depthIterator.data);
            visualType = visualTypeIterator.data;
            break;
        }

        xcb_depth_next(&depthIterator);
    }

    if (!visualType) {
        qCDebug(XWAYLAND) << "No 32-bit visualtype";
        return;
    }

    m_visualId = visualType->visual_id;
    m_colorMap = xcb_generate_id(Xcb::connection());
    xcb_create_colormap(Xcb::connection(), XCB_COLORMAP_ALLOC_NONE,
                        m_colorMap, Xcb::rootWindow(), m_visualId);
}

void XWaylandManager::wmSelection()
{
    m_selRequest.requestor = XCB_NONE;

    quint32 values[1];
    values[0] = XCB_EVENT_MASK_PROPERTY_CHANGE;

    m_selWindow = xcb_generate_id(Xcb::connection());
    xcb_create_window(Xcb::connection(), XCB_COPY_FROM_PARENT,
                      m_selWindow, Xcb::rootWindow(),
                      0, 0, 10, 10, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      Xcb::rootVisual(),
                      XCB_CW_EVENT_MASK, values);
    xcb_set_selection_owner(Xcb::connection(), m_selWindow,
                            Xcb::resources()->atoms->clipboard_manager,
                            XCB_TIME_CURRENT_TIME);

    quint32 mask =
            XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER |
            XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_WINDOW_DESTROY |
            XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_CLIENT_CLOSE;
    xcb_xfixes_select_selection_input(Xcb::connection(),
                                      m_selWindow,
                                      Xcb::resources()->atoms->clipboard,
                                      mask);

    //weston_wm_set_selection
}

void XWaylandManager::initializeDragAndDrop()
{

}

void XWaylandManager::setCursor(xcb_window_t window, const CursorType &cursor)
{
    if (m_lastCursor == cursor)
        return;

    m_lastCursor = cursor;
    quint32 cursorValueList = m_cursors[cursor];
    xcb_change_window_attributes(Xcb::connection(), window,
                                 XCB_CW_CURSOR, &cursorValueList);
    xcb_flush(Xcb::connection());
}

void XWaylandManager::createWindowManager()
{
    static const char name[] = "Green Island the Hawaii compositor";

    m_wmWindow = new Xcb::Window(QRect(0, 0, 10, 10),
                                 XCB_WINDOW_CLASS_INPUT_OUTPUT, 0, Q_NULLPTR);
    xcb_window_t w = m_wmWindow->window();
    m_wmWindow->changeProperty(Xcb::resources()->atoms->net_supporting_wm_check,
                               XCB_ATOM_WINDOW, 32, 1, &w);
    m_wmWindow->changeProperty(Xcb::resources()->atoms->net_wm_name,
                               Xcb::resources()->atoms->utf8_string,
                               8, strlen(name), name);
    xcb_change_property(Xcb::connection(),
                        XCB_PROP_MODE_REPLACE,
                        Xcb::rootWindow(),
                        Xcb::resources()->atoms->net_supporting_wm_check,
                        XCB_ATOM_WINDOW, 32, 1, &w);

    // Claim WM_S0
    xcb_set_selection_owner(Xcb::connection(), w,
                            Xcb::resources()->atoms->wm_s0,
                            XCB_TIME_CURRENT_TIME);
    xcb_set_selection_owner(Xcb::connection(), w,
                            Xcb::resources()->atoms->net_wm_cm_s0,
                            XCB_TIME_CURRENT_TIME);
}

void XWaylandManager::handleButton(xcb_button_press_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_BUTTON_%s (detail %d)",
            event->response_type == XCB_BUTTON_PRESS ? "PRESS" : "RELEASE",
            event->detail);

#if 0 // 2016
    if (!m_windowsMap.contains(event->event))
        return;

    XWaylandWindow *window = m_windowsMap[event->event];
    if (!window->isDecorated())
        return;

    if (event->detail != 1 && event->detail != 2)
        return;

    // TODO:
#endif
}

void XWaylandManager::handleEnter(xcb_enter_notify_event_t *event)
{
    if (!m_windowsMap.contains(event->event))
        return;

#if 0
    XWaylandWindow *window = m_windowsMap[event->event];
    if (!window->isDecorated())
        return;

    // TODO:
    /*
     * location = frame_pointer_enter(window->frame, NULL,
                                       enter->event_x, enter->event_y);
        if (frame_status(window->frame) & FRAME_STATUS_REPAINT)
                weston_wm_window_schedule_repaint(window);

        cursor = get_cursor_for_location(location);
        weston_wm_window_set_cursor(wm, window->frame_id, cursor);
        */
#endif
}

void XWaylandManager::handleLeave(xcb_leave_notify_event_t *event)
{
    if (!m_windowsMap.contains(event->event))
        return;

#if 0
    XWaylandWindow *window = m_windowsMap[event->event];
    if (!window->isDecorated())
        return;

    /*
     * frame_pointer_leave(window->frame, NULL);
        if (frame_status(window->frame) & FRAME_STATUS_REPAINT)
                weston_wm_window_schedule_repaint(window);
                */

    setCursor(window->frameId(), CursorLeftPointer);
#endif
}

void XWaylandManager::handleMotion(xcb_motion_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_MOTION_NOTIFY (window %d, x %d, y %d)",
            event->event, event->event_x, event->event_y);

    if (!m_windowsMap.contains(event->event))
        return;

    XWaylandShellSurface *window = m_windowsMap[event->event];
    window->setX(event->event_x);
    window->setY(event->event_y);
}

void XWaylandManager::handleCreateNotify(xcb_create_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_CREATE_NOTIFY (window %d, width %d, height %d%s%s)",
            event->window, event->width, event->height,
            event->override_redirect ? ", override" : "",
            Xcb::isOurResource(event->window) ? ", ours" : "");

    if (Xcb::isOurResource(event->window))
        return;

    new XWaylandShellSurface(event->window,
                       QRect(QPoint(event->x, event->y), QSize(event->width, event->height)),
                       event->override_redirect != 0, this);
}

void XWaylandManager::handleMapRequest(xcb_map_request_event_t *event)
{
    if (Xcb::isOurResource(event->window)) {
        qCDebug(XWAYLAND_TRACE, "XCB_MAP_REQUEST (window %d, ours)",
                event->window);
        return;
    }

    if (!m_windowsMap.contains(event->window))
        return;

    XWaylandShellSurface *window = m_windowsMap[event->window];
    window->readProperties();

    qCDebug(XWAYLAND_TRACE, "XCB_MAP_REQUEST (window %d, %p)",
            event->window, window);

    window->setWmState(XWaylandShellSurface::NormalState);
    window->setNetWmState();
    window->setWorkspace(0);
    window->map();
}

void XWaylandManager::handleMapNotify(xcb_map_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_MAP_NOTIFY (window %d%s)",
            event->window,
            Xcb::isOurResource(event->window) ? ", ours" : "");
}

void XWaylandManager::handleUnmapNotify(xcb_unmap_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_UNMAP_NOTIFY (window %d, event %d%s)",
            event->window, event->event,
            Xcb::isOurResource(event->window) ? ", ours" : "");

    if (Xcb::isOurResource(event->window))
        return;

    // We just ignore the ICCCM 4.1.4 synthetic unmap notify
    // as it may come in after we've destroyed the window
    if (event->response_type & ~0x80)
        return;

    if (!m_windowsMap.contains(event->window))
        return;

#if 0
    XWaylandWindow *window = m_windowsMap[event->window];
    if (m_focusWindow == window)
        m_focusWindow = Q_NULLPTR;
    window->setSurface(Q_NULLPTR);

    window->setWmState(XWaylandWindow::WithdrawnState);
    window->setWorkspace(-1);
    window->unmap();
#endif
}

void XWaylandManager::handleReparentNotify(xcb_reparent_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_REPARENT_NOTIFY (window %d, parent %d, event %d)",
            event->window, event->parent, event->event);

#if 0
    if (event->parent == Xcb::rootWindow())
        new XWaylandWindow(event->window, QRect(QPoint(event->x, event->y), QSize(10, 10)),
                           event->override_redirect != 0, this);
    else if (!Xcb::isOurResource(event->parent))
        m_windowsMap.take(event->window)->deleteLater();
#endif
}

void XWaylandManager::handleConfigureRequest(xcb_configure_request_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_CONFIGURE_REQUEST (window %d) %d,%d @ %dx%d",
            event->window, event->x, event->y, event->width, event->height);

    if (!m_windowsMap.contains(event->window))
        return;

    XWaylandShellSurface *shellSurface = m_windowsMap[event->window];
    if (!shellSurface->surface())
        return;

    if (shellSurface->fullscreen()) {
        xcb_configure_notify_event_t notify;
        notify.response_type = XCB_CONFIGURE_NOTIFY;
        notify.pad0 = 0;
        notify.event = event->window;
        notify.window = event->window;
        notify.above_sibling = XCB_WINDOW_NONE;
        notify.x = event->x;
        notify.y = event->y;
        notify.width = event->width;
        notify.height = event->height;
        notify.border_width = 0;
        notify.override_redirect = 0;
        notify.pad1 = 0;
        xcb_send_event(Xcb::connection(), 0, event->window, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char *) &notify);
        return;
    }

    int x = 0, y = 0;
    QSize size = shellSurface->surface()->size();

    if (event->value_mask & XCB_CONFIG_WINDOW_WIDTH)
        size.setWidth(event->width);
    if (event->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
        size.setHeight(event->height);

    quint32 i = 0, values[16];
    values[i++] = x;
    values[i++] = y;
    values[i++] = size.width();
    values[i++] = size.height();
    values[i++] = 0;
    quint32 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
            XCB_CONFIG_WINDOW_BORDER_WIDTH;
    if (event->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
        values[i++] = event->sibling;
        mask |= XCB_CONFIG_WINDOW_SIBLING;
    }
    if (event->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
        values[i++] = event->stack_mode;
        mask |= XCB_CONFIG_WINDOW_STACK_MODE;
    }
    xcb_configure_window(Xcb::connection(), event->window, mask, values);
}

void XWaylandManager::handleConfigureNotify(xcb_configure_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_CONFIGURE_NOTIFY (window %d) %d,%d @ %dx%d",
            event->window, event->x, event->y, event->width, event->height);

    if (!m_windowsMap.contains(event->window))
        return;

    XWaylandShellSurface *shellSurface = m_windowsMap[event->window];
    shellSurface->setX(event->x);
    shellSurface->setY(event->y);
    shellSurface->setWidth(event->width);
    shellSurface->setHeight(event->height);
}

void XWaylandManager::handleDestroyNotify(xcb_destroy_notify_event_t *event)
{
    qCDebug(XWAYLAND_TRACE, "XCB_DESTROY_NOTIFY (window %d, event %d%s)",
            event->window, event->event,
            Xcb::isOurResource(event->window) ? ", ours" : "");

    if (Xcb::isOurResource(event->window))
        return;

    if (!m_windowsMap.contains(event->window))
        return;

    m_windowsMap.take(event->window)->deleteLater();
}

void XWaylandManager::handlePropertyNotify(xcb_property_notify_event_t *event)
{
    if (!m_windowsMap.contains(event->window))
        return;

    qCDebug(XWAYLAND_TRACE, "XCB_PROPERTY_NOTIFY (window %d)", event->window);

    XWaylandShellSurface *window = m_windowsMap[event->window];
    if (event->state == XCB_PROPERTY_DELETE)
        qCDebug(XWAYLAND_TRACE, "\tdeleted");
    else
        window->readAndDumpProperty(event->atom);
}

void XWaylandManager::handleClientMessage(xcb_client_message_event_t *event)
{
    QString name = Xcb::Atom::nameFromAtom(event->type);
    qCDebug(XWAYLAND_TRACE, "XCB_CLIENT_MESSAGE (%s %d %d %d %d %d win %d)",
            qPrintable(name),
            event->data.data32[0],
            event->data.data32[1],
            event->data.data32[2],
            event->data.data32[3],
            event->data.data32[4],
            event->window);

    // Check whether we have the surface because the window may get
    // created and destroyed before we actually handle this message
    if (!m_windowsMap.contains(event->window))
        return;

    // Get the surface for this window
    XWaylandShellSurface *window = m_windowsMap[event->window];

    // Handle messages
    if (event->type == Xcb::resources()->atoms->net_wm_moveresize)
        handleMoveResize(window, event);
    else if (event->type == Xcb::resources()->atoms->net_wm_state)
        handleState(window, event);
    else if (event->type == Xcb::resources()->atoms->wl_surface_id)
        handleSurfaceId(window, event);
}

bool XWaylandManager::handleSelection(xcb_generic_event_t *event)
{
    switch (event->response_type & ~0x80) {
    case XCB_SELECTION_NOTIFY:
        handleSelectionNotify((xcb_selection_notify_event_t *)event);
        return true;
        /*
    case XCB_PROPERTY_NOTIFY:
        return handleSelectionPropertyNotify(event);
    case XCB_SELECTION_REQUEST:
        handleSelectionRequest(event);
        return true;
        */
    default:
        break;
    }

    /*
    switch (event->response_type - xfixes->first_event) {
    case XCB_XFIXES_SELECTION_NOTIFY:
        return handleXFixesSelectionNotify(event);
    }
    */

    return false;
}

void XWaylandManager::handleSelectionNotify(xcb_selection_notify_event_t *event)
{
    if (event->property == XCB_ATOM_NONE) {
        /* convert selection failed */
    } else if (event->target == Xcb::resources()->atoms->targets) {
        /*
              weston_wm_get_selection_targets(wm);
      } else {
              weston_wm_get_selection_data(wm);
              */
    }
}

void XWaylandManager::handleMoveResize(XWaylandShellSurface *window, xcb_client_message_event_t *event)
{
    int detail = event->data.data32[2];
    switch (detail) {
    case _NET_WM_MOVERESIZE_MOVE:
        Q_EMIT window->startMove();
        break;
    case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
        Q_EMIT window->startResize(XWaylandShellSurface::TopLeftEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_TOP:
        Q_EMIT window->startResize(XWaylandShellSurface::TopEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
        Q_EMIT window->startResize(XWaylandShellSurface::TopRightEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_RIGHT:
        Q_EMIT window->startResize(XWaylandShellSurface::RightEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
        Q_EMIT window->startResize(XWaylandShellSurface::BottomRightEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
        Q_EMIT window->startResize(XWaylandShellSurface::BottomEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
        Q_EMIT window->startResize(XWaylandShellSurface::BottomLeftEdge);
        break;
    case _NET_WM_MOVERESIZE_SIZE_LEFT:
        Q_EMIT window->startResize(XWaylandShellSurface::LeftEdge);
        break;
    default:
        break;
    }
}

void XWaylandManager::handleState(XWaylandShellSurface *window, xcb_client_message_event_t *event)
{
#if 0
    auto updateState = [](int action, int *state) {
#define _NET_WM_STATE_REMOVE    0
#define _NET_WM_STATE_ADD       1
#define _NET_WM_STATE_TOGGLE    2

        int newState;
        bool changed;

        switch (action) {
        case _NET_WM_STATE_REMOVE:
            newState = 0;
            break;
        case _NET_WM_STATE_ADD:
            newState = 1;
            break;
        case _NET_WM_STATE_TOGGLE:
            newState = !*state;
            break;
        default:
            return false;
        }

        changed = (*state != newState);
        *state = newState;

        return changed;
    };

    quint32 action = event->data.data32[0];
    quint32 property = event->data.data32[1];

    bool maximized = window->isMaximized();

    if (property == Xcb::resources()->atoms->net_wm_state_fullscreen &&
            updateState(action, &window->m_properties.fullscreen)) {
        window->setNetWmState();

        if (window->m_properties.fullscreen) {
            window->m_properties.savedSize = window->m_properties.size;

            if (window->clientWindow())
                window->clientWindow()->setFullScreen(true);
        } else if (window->m_surfaceInterface) {
            window->m_surfaceInterface->setType(QWaylandSurface::Toplevel);
        }
    } else {
        if (property == Xcb::resources()->atoms->net_wm_state_maximized_horz &&
                updateState(action, &window->m_properties.maximizedHorizontally))
            window->setNetWmState();
        if (property == Xcb::resources()->atoms->net_wm_state_maximized_vert &&
                updateState(action, &window->m_properties.maximizedVertically))
            window->setNetWmState();

        if (maximized != window->isMaximized()) {
            if (window->isMaximized()) {
                window->m_properties.savedSize = window->m_properties.size;

                if (window->clientWindow())
                    window->clientWindow()->maximize();
            } else if (window->m_surfaceInterface) {
                window->m_surfaceInterface->setType(QWaylandSurface::Toplevel);
            }
        }
    }
#endif
}

void XWaylandManager::handleSurfaceId(XWaylandShellSurface *window, xcb_client_message_event_t *event)
{
    if (window->surface()) {
        qCWarning(XWAYLAND_TRACE) << "Window" << window->window() << "already has a surface id";
        return;
    }

    quint32 id = event->data.data32[0];
    window->setSurfaceId(id);

    wl_resource *resource = wl_client_get_object(m_server->client(), id);
    if (resource) {
        window->setSurface(QWaylandSurface::fromResource(resource));
    } else {
        window->setSurface(Q_NULLPTR);
        m_unpairedWindows.append(window);
    }
}

void XWaylandManager::wmEvents()
{
    // Uncomment if you want to be flooded by messages
    //qCDebug(XWAYLAND_TRACE) << "WM event";

    xcb_generic_event_t *event;

    while ((event = xcb_poll_for_event(Xcb::connection()))) {
        //handle selection event
        //handle dnd event

        int type = event->response_type & ~0x80;
        switch (type) {
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
            handleButton((xcb_button_press_event_t *)event);
            break;
        case XCB_ENTER_NOTIFY:
            handleEnter((xcb_enter_notify_event_t *)event);
            break;
        case XCB_LEAVE_NOTIFY:
            handleLeave((xcb_leave_notify_event_t *)event);
            break;
        case XCB_MOTION_NOTIFY:
            handleMotion((xcb_motion_notify_event_t *)event);
            break;
        case XCB_CREATE_NOTIFY:
            handleCreateNotify((xcb_create_notify_event_t *)event);
            break;
        case XCB_MAP_REQUEST:
            handleMapRequest((xcb_map_request_event_t *)event);
            break;
        case XCB_MAP_NOTIFY:
            handleMapNotify((xcb_map_notify_event_t *)event);
            break;
        case XCB_UNMAP_NOTIFY:
            handleUnmapNotify((xcb_unmap_notify_event_t *)event);
            break;
        case XCB_REPARENT_NOTIFY:
            handleReparentNotify((xcb_reparent_notify_event_t *)event);
            break;
        case XCB_CONFIGURE_REQUEST:
            handleConfigureRequest((xcb_configure_request_event_t *)event);
            break;
        case XCB_CONFIGURE_NOTIFY:
            handleConfigureNotify((xcb_configure_notify_event_t *)event);
            break;
        case XCB_DESTROY_NOTIFY:
            handleDestroyNotify((xcb_destroy_notify_event_t *)event);
            break;
        case XCB_MAPPING_NOTIFY:
            break;
        case XCB_PROPERTY_NOTIFY:
            handlePropertyNotify((xcb_property_notify_event_t *)event);
            break;
        case XCB_CLIENT_MESSAGE:
            handleClientMessage((xcb_client_message_event_t *)event);
            break;
        default:
            break;
        }

        free(event);
    }

    xcb_flush(Xcb::connection());
}
