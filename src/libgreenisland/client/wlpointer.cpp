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

#include "wlpointer.h"
#include "wlseat.h"

#include <wayland-cursor.h>

namespace GreenIsland {

namespace Client {

WlPointer::WlPointer(WlSeat *seat)
    : QtWayland::wl_pointer()
    , m_seat(seat)
    , m_enterSerial(0)
{
    // Create a surface to hold the cursor image
    m_cursorSurface = wl_compositor_create_surface(seat->compositor());
}

WlPointer::~WlPointer()
{
    if (m_cursorSurface)
        wl_surface_destroy(m_cursorSurface);

    if (m_seat->version() >= 3)
        release();
    else
        wl_pointer_destroy(object());
}

void WlPointer::setCursor(wl_cursor_image *image)
{
    // Hide the cursor when no image is provided
    if (!image) {
        set_cursor(m_enterSerial, Q_NULLPTR, 0, 0);
        return;
    }

    // Get buffer
    wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    if (!buffer) {
        // Hide the cursor when no buffer is provided
        set_cursor(m_enterSerial, Q_NULLPTR, 0, 0);
        return;
    }

    // Set cursor surface
    set_cursor(m_enterSerial, m_cursorSurface, image->hotspot_x, image->hotspot_y);

    // Attach cursor buffer to surface
    wl_surface_attach(m_cursorSurface, buffer, 0, 0);
    wl_surface_damage(m_cursorSurface, 0, 0, image->width, image->height);
    wl_surface_commit(m_cursorSurface);
}

void WlPointer::pointer_enter(uint32_t serial, wl_surface *surface,
                              wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    Q_UNUSED(surface)
    Q_UNUSED(surface_x)
    Q_UNUSED(surface_y)

    m_enterSerial = serial;
}

} // namespace Client

} // namespace GreenIsland
