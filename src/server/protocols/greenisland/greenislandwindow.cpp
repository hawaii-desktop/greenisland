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

#include "greenislandwindow.h"

namespace GreenIsland {

GreenIslandWindow::GreenIslandWindow(wl_client *client, ClientWindow *window)
    : QtWaylandServer::greenisland_window(client, 0, 1)
    , m_window(window)
    , m_state(QtWaylandServer::greenisland_windows::state_inactive)
{
    determineState();

    QObject::connect(window, &ClientWindow::titleChanged, [this]() {
        if (m_window)
            send_title_changed(m_window->title());
    });
    QObject::connect(window, &ClientWindow::appIdChanged, [this]() {
        if (m_window)
            send_app_id_changed(m_window->appId());
    });
    QObject::connect(window, &ClientWindow::activeChanged, [this]() {
        if (m_window) {
            determineState();
            send_state_changed(m_state);
        }
    });
    QObject::connect(window, &ClientWindow::minimizedChanged, [this]() {
        if (m_window) {
            determineState();
            send_state_changed(m_state);
        }
    });
    QObject::connect(window, &ClientWindow::maximizedChanged, [this]() {
        if (m_window) {
            determineState();
            send_state_changed(m_state);
        }
    });
    QObject::connect(window, &ClientWindow::fullScreenChanged, [this]() {
        if (m_window) {
            determineState();
            send_state_changed(m_state);
        }
    });
}

GreenIslandWindow::~GreenIslandWindow()
{
    send_unmapped();
}

ClientWindow *GreenIslandWindow::window() const
{
    return m_window;
}

uint32_t GreenIslandWindow::state() const
{
    return m_state;
}

QtWaylandServer::greenisland_windows::type GreenIslandWindow::type2WlType(ClientWindow::Type type)
{
    switch (type) {
    case ClientWindow::Popup:
        return QtWaylandServer::greenisland_windows::type_popup;
    case ClientWindow::Transient:
        return QtWaylandServer::greenisland_windows::type_transient;
    default:
        break;
    }

    return QtWaylandServer::greenisland_windows::type_toplevel;
}

void GreenIslandWindow::determineState()
{
    m_state = QtWaylandServer::greenisland_windows::state_inactive;

    if (m_window->isActive())
        m_state |= QtWaylandServer::greenisland_windows::state_active;
    else
        m_state ^= QtWaylandServer::greenisland_windows::state_active;

    if (m_window->isMinimized())
        m_state |= QtWaylandServer::greenisland_windows::state_minimized;
    else
        m_state ^= QtWaylandServer::greenisland_windows::state_minimized;

    if (m_window->isMaximized())
        m_state |= QtWaylandServer::greenisland_windows::state_maximized;
    else
        m_state ^= QtWaylandServer::greenisland_windows::state_maximized;

    if (m_window->isFullScreen())
        m_state |= QtWaylandServer::greenisland_windows::state_fullscreen;
    else
        m_state ^= QtWaylandServer::greenisland_windows::state_fullscreen;
}

}
