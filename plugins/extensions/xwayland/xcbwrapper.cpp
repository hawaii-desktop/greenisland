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

#include <QtCore/qglobal.h>

#include "xcbwrapper.h"
#include "xcbresources.h"

namespace Xcb {

static xcb_connection_t *s_connection = Q_NULLPTR;
static xcb_screen_t *s_screen = Q_NULLPTR;
Resources *s_resources = Q_NULLPTR;

xcb_connection_t *connection()
{
    return s_connection;
}

xcb_screen_t *screen()
{
    return s_screen;
}

xcb_window_t rootWindow()
{
    if (!s_screen)
        return XCB_WINDOW_NONE;
    return s_screen->root;
}

xcb_visualid_t rootVisual()
{
    if (!s_screen)
        return 0;
    return s_screen->root_visual;
}

Resources *resources()
{
    if (!s_resources)
        s_resources = new Resources();
    return s_resources;
}

void openConnection(int fd)
{
    if (s_connection)
        return;
    s_connection = xcb_connect_to_fd(fd, Q_NULLPTR);

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(s_connection));
    s_screen = iter.data;
}

void closeConnection()
{
    if (s_connection) {
        delete s_resources;
        xcb_disconnect(s_connection);
        s_connection = Q_NULLPTR;
    }
}

bool isOurResource(quint32 id)
{
    if (!s_connection)
        return false;
    const xcb_setup_t *setup = xcb_get_setup(s_connection);
    return (id & ~setup->resource_id_mask) == setup->resource_id_base;
}

} // namespace Xcb

