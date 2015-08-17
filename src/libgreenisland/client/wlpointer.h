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

#ifndef GREENISLAND_CLIENT_WLPOINTER_H
#define GREENISLAND_CLIENT_WLPOINTER_H

#include "qwayland-wayland.h"

struct wl_cursor_image;

namespace GreenIsland {

namespace Client {

class WlSeat;

class WlPointer : public QtWayland::wl_pointer
{
public:
    WlPointer(WlSeat *seat);
    ~WlPointer();

    void setCursor(wl_cursor_image *image);

protected:
    void pointer_enter(uint32_t serial, wl_surface *surface,
                       wl_fixed_t surface_x, wl_fixed_t surface_y) Q_DECL_OVERRIDE;

private:
    WlSeat *m_seat;
    wl_surface *m_cursorSurface;
    quint32 m_enterSerial;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLAND_CLIENT_WLPOINTER_H
