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

#ifndef GREENISLANDWINDOW_H
#define GREENISLANDWINDOW_H

#include <QtCore/QPointer>
#include <QtCompositor/QWaylandGlobalInterface>

#include "clientwindow.h"
#include "qwayland-server-greenisland.h"

namespace GreenIsland {

class GreenIslandWindow : public QtWaylandServer::greenisland_window
{
public:
    GreenIslandWindow(wl_client *client, ClientWindow *window);
    ~GreenIslandWindow();

    ClientWindow *window() const;

    uint32_t state() const;

    static QtWaylandServer::greenisland_windows::type type2WlType(ClientWindow::Type type);

private:
    QPointer<ClientWindow> m_window;
    uint32_t m_state;

    void determineState();
};

}

#endif // GREENISLANDWINDOW_H
