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

#include "xcbcursors.h"
#include "xcbwrapper.h"
#include "xcbresources.h"

#include <X11/Xcursor/Xcursor.h>

/*
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

namespace Xcb {

namespace Cursors {

/*
 * The following correspondences between file names and cursors was copied
 * from: https://bugs.kde.org/attachment.cgi?id=67313
 */

static const char *bottom_left_corners[] = {
    "bottom_left_corner",
    "sw-resize",
    "size_bdiag"
};

static const char *bottom_right_corners[] = {
    "bottom_right_corner",
    "se-resize",
    "size_fdiag"
};

static const char *bottom_sides[] = {
    "bottom_side",
    "s-resize",
    "size_ver"
};

static const char *left_ptrs[] = {
    "left_ptr",
    "default",
    "top_left_arrow",
    "left-arrow"
};

static const char *left_sides[] = {
    "left_side",
    "w-resize",
    "size_hor"
};

static const char *right_sides[] = {
    "right_side",
    "e-resize",
    "size_hor"
};

static const char *top_left_corners[] = {
    "top_left_corner",
    "nw-resize",
    "size_fdiag"
};

static const char *top_right_corners[] = {
    "top_right_corner",
    "ne-resize",
    "size_bdiag"
};

static const char *top_sides[] = {
    "top_side",
    "n-resize",
    "size_ver"
};

struct cursor_alternatives {
    const char **names;
    size_t count;
};

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

#define CURSOR_ENTRY(x) {(x), ARRAY_LENGTH((x))}

static const struct cursor_alternatives cursors[] = {
    CURSOR_ENTRY(top_sides),
    CURSOR_ENTRY(bottom_sides),
    CURSOR_ENTRY(left_sides),
    CURSOR_ENTRY(right_sides),
    CURSOR_ENTRY(top_left_corners),
    CURSOR_ENTRY(top_right_corners),
    CURSOR_ENTRY(bottom_left_corners),
    CURSOR_ENTRY(bottom_right_corners),
    CURSOR_ENTRY(left_ptrs)
};

static xcb_cursor_t
xcb_cursor_image_load_cursor(const XcursorImage *img)
{
    xcb_connection_t *c = Xcb::connection();
    xcb_screen_iterator_t s = xcb_setup_roots_iterator(xcb_get_setup(c));
    xcb_screen_t *screen = s.data;
    xcb_gcontext_t gc;
    xcb_pixmap_t pix;
    xcb_render_picture_t pic;
    xcb_cursor_t cursor;
    int stride = img->width * 4;

    pix = xcb_generate_id(c);
    xcb_create_pixmap(c, 32, pix, screen->root, img->width, img->height);

    pic = xcb_generate_id(c);
    xcb_render_create_picture(c, pic, pix, Xcb::resources()->formatRgba.id, 0, 0);

    gc = xcb_generate_id(c);
    xcb_create_gc(c, gc, pix, 0, 0);

    xcb_put_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pix, gc,
                  img->width, img->height, 0, 0, 0, 32,
                  stride * img->height, (uint8_t *)img->pixels);
    xcb_free_gc(c, gc);

    cursor = xcb_generate_id(c);
    xcb_render_create_cursor(c, cursor, pic, img->xhot, img->yhot);

    xcb_render_free_picture(c, pic);
    xcb_free_pixmap(c, pix);

    return cursor;
}

static xcb_cursor_t
xcb_cursor_images_load_cursor(const XcursorImages *images)
{
    // TODO: treat animated cursors as well
    if (images->nimage != 1)
        return -1;

    return xcb_cursor_image_load_cursor(images->images[0]);
}

static xcb_cursor_t
xcb_cursor_library_load_cursor(const char *file)
{
    xcb_cursor_t cursor;
    XcursorImages *images;
    char *v = NULL;
    int size = 0;

    if (!file)
        return 0;

    v = getenv("XCURSOR_SIZE");
    if (v)
        size = atoi(v);

    if (!size)
        size = 32;

    images = XcursorLibraryLoadImages(file, Q_NULLPTR, size);
    if (!images)
        return -1;

    cursor = xcb_cursor_images_load_cursor(images);
    XcursorImagesDestroy(images);

    return cursor;
}

xcb_cursor_t *createCursors()
{
    const char *name;
    int count = ARRAY_LENGTH(cursors);

    xcb_cursor_t *c = (xcb_cursor_t *)malloc(count * sizeof(xcb_cursor_t));

    for (int i = 0; i < count; i++) {
        for (size_t j = 0; j < cursors[i].count; j++) {
            name = cursors[i].names[j];
            c[i] = xcb_cursor_library_load_cursor(name);
            if (c[i] != (xcb_cursor_t)-1)
                break;
        }
    }

    return c;
}

void destroyCursors(xcb_cursor_t *c)
{
    if (!c)
        return;

    for (quint8 i = 0; i < ARRAY_LENGTH(c); i++)
        xcb_free_cursor(Xcb::connection(), c[i]);

    free(c);
}


} // namespace Cursors

} // namespace Xcb

