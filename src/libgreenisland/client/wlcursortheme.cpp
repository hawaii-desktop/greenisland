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

#include "compositor.h"
#include "compositor_p.h"
#include "wlcursortheme.h"
#include "wlpointer.h"
#include "wlseat.h"
#include "wlshmpool.h"

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-server.h>

Q_LOGGING_CATEGORY(WLCURSORTHEME, "greenisland.wlcursortheme")

namespace GreenIsland {

namespace Client {

/*
 * WlCursorThemePrivate
 */

class WlCursorThemePrivate
{
public:
    WlCursorThemePrivate(WlShmPool *pool, WlSeat *seat)
        : pool(pool)
        , seat(seat)
        , theme(Q_NULLPTR)
    {
        // Cursor theme name
        cursorThemeName = QString::fromUtf8(qgetenv("XCURSOR_THEME"));
        if (cursorThemeName.isEmpty())
            cursorThemeName = QStringLiteral("default");

        // Cursor size
        QByteArray cursorSizeFromEnv = qgetenv("XCURSOR_SIZE");
        bool hasCursorSize = false;
        cursorSize = cursorSizeFromEnv.toInt(&hasCursorSize);
        if (!hasCursorSize || cursorSize <= 0)
            cursorSize = 32;

        // Associate cursor shapes with names
        fillCursorShapes();
    }

    ~WlCursorThemePrivate()
    {
        destroyTheme();
    }

    void fillCursorShapes()
    {
        cursorShapes.insert(WlCursorTheme::ArrowCursor, "left_ptr");
        cursorShapes.insert(WlCursorTheme::ArrowCursor, "default");
        cursorShapes.insert(WlCursorTheme::ArrowCursor, "top_left_arrow");
        cursorShapes.insert(WlCursorTheme::ArrowCursor, "left_arrow");

        cursorShapes.insert(WlCursorTheme::UpArrowCursor, "up_arrow");

        cursorShapes.insert(WlCursorTheme::CrossCursor, "cross");

        cursorShapes.insert(WlCursorTheme::WaitCursor, "wait");
        cursorShapes.insert(WlCursorTheme::WaitCursor, "watch");
        cursorShapes.insert(WlCursorTheme::WaitCursor, "0426c94ea35c87780ff01dc239897213");

        cursorShapes.insert(WlCursorTheme::IBeamCursor, "ibeam");
        cursorShapes.insert(WlCursorTheme::IBeamCursor, "text");
        cursorShapes.insert(WlCursorTheme::IBeamCursor, "xterm");

        cursorShapes.insert(WlCursorTheme::SizeVerCursor, "size_ver");
        cursorShapes.insert(WlCursorTheme::SizeVerCursor, "ns-resize");
        cursorShapes.insert(WlCursorTheme::SizeVerCursor, "v_double_arrow");
        cursorShapes.insert(WlCursorTheme::SizeVerCursor, "00008160000006810000408080010102");

        cursorShapes.insert(WlCursorTheme::SizeHorCursor, "size_hor");
        cursorShapes.insert(WlCursorTheme::SizeHorCursor, "ew-resize");
        cursorShapes.insert(WlCursorTheme::SizeHorCursor, "h_double_arrow");
        cursorShapes.insert(WlCursorTheme::SizeHorCursor, "028006030e0e7ebffc7f7070c0600140");

        cursorShapes.insert(WlCursorTheme::SizeBDiagCursor, "size_bdiag");
        cursorShapes.insert(WlCursorTheme::SizeBDiagCursor, "nesw-resize");
        cursorShapes.insert(WlCursorTheme::SizeBDiagCursor, "50585d75b494802d0151028115016902");
        cursorShapes.insert(WlCursorTheme::SizeBDiagCursor, "fcf1c3c7cd4491d801f1e1c78f100000");

        cursorShapes.insert(WlCursorTheme::SizeFDiagCursor, "size_fdiag");
        cursorShapes.insert(WlCursorTheme::SizeFDiagCursor, "nwse-resize");
        cursorShapes.insert(WlCursorTheme::SizeFDiagCursor, "38c5dff7c7b8962045400281044508d2");
        cursorShapes.insert(WlCursorTheme::SizeFDiagCursor, "c7088f0f3e6c8088236ef8e1e3e70000");

        cursorShapes.insert(WlCursorTheme::SizeAllCursor, "size_all");

        cursorShapes.insert(WlCursorTheme::SplitVCursor, "split_v");
        cursorShapes.insert(WlCursorTheme::SplitVCursor, "row-resize");
        cursorShapes.insert(WlCursorTheme::SplitVCursor, "sb_v_double_arrow");
        cursorShapes.insert(WlCursorTheme::SplitVCursor, "2870a09082c103050810ffdffffe0204");
        cursorShapes.insert(WlCursorTheme::SplitVCursor, "c07385c7190e701020ff7ffffd08103c");

        cursorShapes.insert(WlCursorTheme::SplitHCursor, "split_h");
        cursorShapes.insert(WlCursorTheme::SplitHCursor, "col-resize");
        cursorShapes.insert(WlCursorTheme::SplitHCursor, "sb_h_double_arrow");
        cursorShapes.insert(WlCursorTheme::SplitHCursor, "043a9f68147c53184671403ffa811cc5");
        cursorShapes.insert(WlCursorTheme::SplitHCursor, "14fef782d02440884392942c11205230");

        cursorShapes.insert(WlCursorTheme::PointingHandCursor, "pointing_hand");
        cursorShapes.insert(WlCursorTheme::PointingHandCursor, "pointer");
        cursorShapes.insert(WlCursorTheme::PointingHandCursor, "hand1");
        cursorShapes.insert(WlCursorTheme::PointingHandCursor, "e29285e634086352946a0e7090d73106");

        cursorShapes.insert(WlCursorTheme::ForbiddenCursor, "forbidden");
        cursorShapes.insert(WlCursorTheme::ForbiddenCursor, "not-allowed");
        cursorShapes.insert(WlCursorTheme::ForbiddenCursor, "crossed_circle");
        cursorShapes.insert(WlCursorTheme::ForbiddenCursor, "circle");
        cursorShapes.insert(WlCursorTheme::ForbiddenCursor, "03b6e0fcb3499374a867c041f52298f0");

        cursorShapes.insert(WlCursorTheme::WhatsThisCursor, "whats_this");
        cursorShapes.insert(WlCursorTheme::WhatsThisCursor, "help");
        cursorShapes.insert(WlCursorTheme::WhatsThisCursor, "question_arrow");
        cursorShapes.insert(WlCursorTheme::WhatsThisCursor, "5c6cd98b3f3ebcb1f9c7f1c204630408");
        cursorShapes.insert(WlCursorTheme::WhatsThisCursor, "d9ce0ab605698f320427677b458ad60b");

        cursorShapes.insert(WlCursorTheme::BusyCursor, "left_ptr_watch");
        cursorShapes.insert(WlCursorTheme::BusyCursor, "half-busy");
        cursorShapes.insert(WlCursorTheme::BusyCursor, "progress");
        cursorShapes.insert(WlCursorTheme::BusyCursor, "00000000000000020006000e7e9ffc3f");
        cursorShapes.insert(WlCursorTheme::BusyCursor, "08e8e1c95fe2fc01f976f1e063a24ccd");

        cursorShapes.insert(WlCursorTheme::OpenHandCursor, "openhand");
        cursorShapes.insert(WlCursorTheme::OpenHandCursor, "fleur");
        cursorShapes.insert(WlCursorTheme::OpenHandCursor, "5aca4d189052212118709018842178c0");
        cursorShapes.insert(WlCursorTheme::OpenHandCursor, "9d800788f1b08800ae810202380a0822");

        cursorShapes.insert(WlCursorTheme::ClosedHandCursor, "closedhand");
        cursorShapes.insert(WlCursorTheme::ClosedHandCursor, "grabbing");
        cursorShapes.insert(WlCursorTheme::ClosedHandCursor, "208530c400c041818281048008011002");

        cursorShapes.insert(WlCursorTheme::DragCopyCursor, "dnd-copy");
        cursorShapes.insert(WlCursorTheme::DragCopyCursor, "copy");

        cursorShapes.insert(WlCursorTheme::DragMoveCursor, "dnd-move");
        cursorShapes.insert(WlCursorTheme::DragMoveCursor, "move");

        cursorShapes.insert(WlCursorTheme::DragLinkCursor, "dnd-link");
        cursorShapes.insert(WlCursorTheme::DragLinkCursor, "link");

        cursorShapes.insert(WlCursorTheme::ResizeNorthCursor, "n-resize");
        cursorShapes.insert(WlCursorTheme::ResizeNorthCursor, "top_side");

        cursorShapes.insert(WlCursorTheme::ResizeSouthCursor, "s-resize");
        cursorShapes.insert(WlCursorTheme::ResizeSouthCursor, "bottom_side");

        cursorShapes.insert(WlCursorTheme::ResizeEastCursor, "e-resize");
        cursorShapes.insert(WlCursorTheme::ResizeEastCursor, "right_side");

        cursorShapes.insert(WlCursorTheme::ResizeWestCursor, "w-resize");
        cursorShapes.insert(WlCursorTheme::ResizeWestCursor, "left_side");

        cursorShapes.insert(WlCursorTheme::ResizeNorthWestCursor, "nw-resize");
        cursorShapes.insert(WlCursorTheme::ResizeNorthWestCursor, "top_left_corner");

        cursorShapes.insert(WlCursorTheme::ResizeSouthEastCursor, "se-resize");
        cursorShapes.insert(WlCursorTheme::ResizeSouthEastCursor, "bottom_right_corner");

        cursorShapes.insert(WlCursorTheme::ResizeNorthEastCursor, "ne-resize");
        cursorShapes.insert(WlCursorTheme::ResizeNorthEastCursor, "top_right_corner");

        cursorShapes.insert(WlCursorTheme::ResizeSouthWestCursor, "sw-resize");
        cursorShapes.insert(WlCursorTheme::ResizeSouthWestCursor, "bottom_left_corner");
    }

    void loadTheme()
    {
        if (!pool->isValid())
            return;
        if (theme)
            return;

        theme = wl_cursor_theme_load(cursorThemeName.toUtf8().constData(),
                                     cursorSize, pool->shm());
        if (theme)
            qCDebug(WLCURSORTHEME) << "Cursor theme" << cursorThemeName << "loaded";
        else
            qCWarning(WLCURSORTHEME) << "Unable to load cursor theme" << cursorThemeName;
    }

    void destroyTheme()
    {
        if (!theme)
            return;

        wl_cursor_theme_destroy(theme);
        theme = Q_NULLPTR;
    }

    wl_cursor *requestCursor(WlCursorTheme::CursorShape shape)
    {
        // Retrieve a cached cursor
        wl_cursor *cursor = cursors.value(shape, Q_NULLPTR);

        // Or load it for the first time
        if (!cursor) {
            if (!theme)
                return Q_NULLPTR;

            QList<QByteArray> cursorNames = cursorShapes.values(shape);
            Q_FOREACH (const QByteArray &name, cursorNames) {
                cursor = wl_cursor_theme_get_cursor(theme, name.constData());
                if (cursor) {
                    cursors.insert(shape, cursor);
                    break;
                }
            }
        }

        // If we still have no cursor, fall back to default cursor
        if (!cursor && shape != WlCursorTheme::ArrowCursor)
            cursor = requestCursor(WlCursorTheme::ArrowCursor);

        return cursor;
    }

    WlShmPool *pool;
    WlSeat *seat;
    QString cursorThemeName;
    int cursorSize;
    wl_cursor_theme *theme;
    QMultiMap<WlCursorTheme::CursorShape, QByteArray> cursorShapes;
    QMap<WlCursorTheme::CursorShape, wl_cursor *> cursors;
};

/*
 * WlCursorTheme
 */

WlCursorTheme::WlCursorTheme(WlShmPool *pool, WlSeat *seat)
    : d(new WlCursorThemePrivate(pool, seat))
{
}

WlCursorTheme::~WlCursorTheme()
{
    delete d;
}

wl_cursor_image *WlCursorTheme::cursorImage(CursorShape shape)
{
    // Try to load the theme
    d->loadTheme();
    if (!d->theme)
        return Q_NULLPTR;

    wl_cursor *cursor = d->requestCursor(shape);
    if (!cursor || cursor->image_count <= 0)
        return Q_NULLPTR;
    return cursor->images[0];
}

void WlCursorTheme::changeCursor(CursorShape shape)
{
    // Unset cursor if a blank shape is requested
    if (shape == BlankCursor && d->seat->pointer()) {
        d->seat->pointer()->setCursor(Q_NULLPTR);
        return;
    }

    // Find cursor image for the requested shape
    wl_cursor_image *image = cursorImage(shape);
    if (!image) {
        qCWarning(WLCURSORTHEME) << "No image for shape" << shape;
        return;
    }

    // Set cursor surface and hotspot (only if the shape has changed)
    if (d->seat->pointer())
        d->seat->pointer()->setCursor(image);
}

} // namespace Client

} // namespace GreenIsland
