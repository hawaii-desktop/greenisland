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

#ifndef XCB_WRAPPER_H
#define XCB_WRAPPER_H

#include <QtCore/qglobal.h>

#include <xcb/xcb.h>

namespace Xcb {

class Resources;

xcb_connection_t *connection();
xcb_screen_t *screen();
xcb_window_t rootWindow();
xcb_visualid_t rootVisual();
Resources *resources();

void openConnection(int fd);
void closeConnection();

bool isOurResource(quint32 id);

} // namespace Xcb

#endif // XCB_WRAPPER_H
