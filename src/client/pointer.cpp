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

#include "pointer.h"
#include "pointer_p.h"
#include "seat.h"

namespace GreenIsland {

namespace Client {

/*
 * PointerPrivate
 */

PointerPrivate::PointerPrivate()
    : QtWayland::wl_pointer()
    , seat(Q_NULLPTR)
    , enterSerial(0)
{
}

PointerPrivate::~PointerPrivate()
{
    if (cursorSurface)
        wl_surface_destroy(cursorSurface);

    if (seat->version() >= 3)
        release();
    else
        wl_pointer_destroy(object());
}

void PointerPrivate::pointer_enter(uint32_t serial, wl_surface *surface,
                                   wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    Q_UNUSED(surface);
    Q_UNUSED(surface_x);
    Q_UNUSED(surface_y);

    enterSerial = serial;
}

/*
 * Pointer
 */

Pointer::Pointer(Seat *seat)
    : QObject(*new PointerPrivate(), seat)
{
    // Create a surface to hold the cursor image
    d_func()->seat = seat;
    d_func()->cursorSurface = wl_compositor_create_surface(seat->compositor());
}

void Pointer::setCursor(wl_cursor_image *image)
{
    Q_D(Pointer);

    // Hide the cursor when no image is provided
    if (!image) {
        d->set_cursor(d->enterSerial, Q_NULLPTR, 0, 0);
        return;
    }

    // Get buffer
    wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    if (!buffer) {
        // Hide the cursor when no buffer is provided
        d->set_cursor(d->enterSerial, Q_NULLPTR, 0, 0);
        return;
    }

    // Set cursor surface
    d->set_cursor(d->enterSerial, d->cursorSurface,
                  image->hotspot_x, image->hotspot_y);

    // Attach cursor buffer to surface
    wl_surface_attach(d->cursorSurface, buffer, 0, 0);
    wl_surface_damage(d->cursorSurface, 0, 0, image->width, image->height);
    wl_surface_commit(d->cursorSurface);
}

} // namespace Client

} // namespace GreenIsland

#include "moc_pointer.cpp"
