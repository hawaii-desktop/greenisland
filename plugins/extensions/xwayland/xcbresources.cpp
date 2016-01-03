/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include "xcbwrapper.h"
#include "xcbresources.h"
#include "xwayland.h"

#include <xcb/xfixes.h>
#include <xcb/composite.h>

namespace Xcb {

Resources::Resources()
{
    xcb_prefetch_extension_data(connection(), &xcb_xfixes_id);
    xcb_prefetch_extension_data(connection(), &xcb_composite_id);

    xcb_render_query_pict_formats_cookie_t formatsCookie =
            xcb_render_query_pict_formats(connection());

    atoms = new Xcb::Atoms();

    xfixes = xcb_get_extension_data(connection(), &xcb_xfixes_id);
    if (!xfixes || !xfixes->present)
        qCWarning(XWAYLAND) << "xfixes not available";

    xcb_xfixes_query_version_cookie_t xfixesCookie =
            xcb_xfixes_query_version(connection(),
                                     XCB_XFIXES_MAJOR_VERSION,
                                     XCB_XFIXES_MINOR_VERSION);
    xcb_xfixes_query_version_reply_t *xfixesReply =
            xcb_xfixes_query_version_reply(connection(), xfixesCookie, Q_NULLPTR);

    qCDebug(XWAYLAND, "xfixes version: %d.%d",
            xfixesReply->major_version,
            xfixesReply->minor_version);

    free(xfixesReply);

    xcb_render_query_pict_formats_reply_t *formatsReply =
            xcb_render_query_pict_formats_reply(connection(), formatsCookie, 0);
    if (formatsReply) {
        xcb_render_pictforminfo_t *formats =
                xcb_render_query_pict_formats_formats(formatsReply);
        for (quint32 i = 0; i < formatsReply->num_formats; i++) {
            if (formats[i].direct.red_mask != 0xff &&
                    formats[i].direct.red_shift != 16)
                continue;
            if (formats[i].type == XCB_RENDER_PICT_TYPE_DIRECT &&
                    formats[i].depth == 24)
                formatRgb = formats[i];
            if (formats[i].type == XCB_RENDER_PICT_TYPE_DIRECT &&
                    formats[i].depth == 32 &&
                    formats[i].direct.alpha_mask == 0xff &&
                    formats[i].direct.alpha_shift == 24)
                formatRgba = formats[i];
        }

        free(formatsReply);
    }
}

Resources::~Resources()
{
    delete atoms;
}

} // namespace Xcb

