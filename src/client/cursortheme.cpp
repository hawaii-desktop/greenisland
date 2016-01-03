/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "buffer_p.h"
#include "compositor.h"
#include "cursortheme.h"
#include "cursortheme_p.h"
#include "pointer.h"
#include "seat.h"
#include "shm_p.h"
#include "shmpool.h"

Q_LOGGING_CATEGORY(WLCURSORTHEME, "greenisland.client.cursortheme")

namespace GreenIsland {

namespace Client {

/*
 * CursorThemePrivate
 */

CursorThemePrivate::CursorThemePrivate()
    : compositor(Q_NULLPTR)
    , pool(Q_NULLPTR)
    , seat(Q_NULLPTR)
    , cursorSurface(Q_NULLPTR)
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

CursorThemePrivate::~CursorThemePrivate()
{
    destroyTheme();
}

void CursorThemePrivate::fillCursorShapes()
{
    cursorShapes.insert(CursorTheme::ArrowCursor, "left_ptr");
    cursorShapes.insert(CursorTheme::ArrowCursor, "default");
    cursorShapes.insert(CursorTheme::ArrowCursor, "top_left_arrow");
    cursorShapes.insert(CursorTheme::ArrowCursor, "left_arrow");

    cursorShapes.insert(CursorTheme::UpArrowCursor, "up_arrow");

    cursorShapes.insert(CursorTheme::CrossCursor, "cross");

    cursorShapes.insert(CursorTheme::WaitCursor, "wait");
    cursorShapes.insert(CursorTheme::WaitCursor, "watch");
    cursorShapes.insert(CursorTheme::WaitCursor, "0426c94ea35c87780ff01dc239897213");

    cursorShapes.insert(CursorTheme::IBeamCursor, "ibeam");
    cursorShapes.insert(CursorTheme::IBeamCursor, "text");
    cursorShapes.insert(CursorTheme::IBeamCursor, "xterm");

    cursorShapes.insert(CursorTheme::SizeVerCursor, "size_ver");
    cursorShapes.insert(CursorTheme::SizeVerCursor, "ns-resize");
    cursorShapes.insert(CursorTheme::SizeVerCursor, "v_double_arrow");
    cursorShapes.insert(CursorTheme::SizeVerCursor, "00008160000006810000408080010102");

    cursorShapes.insert(CursorTheme::SizeHorCursor, "size_hor");
    cursorShapes.insert(CursorTheme::SizeHorCursor, "ew-resize");
    cursorShapes.insert(CursorTheme::SizeHorCursor, "h_double_arrow");
    cursorShapes.insert(CursorTheme::SizeHorCursor, "028006030e0e7ebffc7f7070c0600140");

    cursorShapes.insert(CursorTheme::SizeBDiagCursor, "size_bdiag");
    cursorShapes.insert(CursorTheme::SizeBDiagCursor, "nesw-resize");
    cursorShapes.insert(CursorTheme::SizeBDiagCursor, "50585d75b494802d0151028115016902");
    cursorShapes.insert(CursorTheme::SizeBDiagCursor, "fcf1c3c7cd4491d801f1e1c78f100000");

    cursorShapes.insert(CursorTheme::SizeFDiagCursor, "size_fdiag");
    cursorShapes.insert(CursorTheme::SizeFDiagCursor, "nwse-resize");
    cursorShapes.insert(CursorTheme::SizeFDiagCursor, "38c5dff7c7b8962045400281044508d2");
    cursorShapes.insert(CursorTheme::SizeFDiagCursor, "c7088f0f3e6c8088236ef8e1e3e70000");

    cursorShapes.insert(CursorTheme::SizeAllCursor, "size_all");

    cursorShapes.insert(CursorTheme::SplitVCursor, "split_v");
    cursorShapes.insert(CursorTheme::SplitVCursor, "row-resize");
    cursorShapes.insert(CursorTheme::SplitVCursor, "sb_v_double_arrow");
    cursorShapes.insert(CursorTheme::SplitVCursor, "2870a09082c103050810ffdffffe0204");
    cursorShapes.insert(CursorTheme::SplitVCursor, "c07385c7190e701020ff7ffffd08103c");

    cursorShapes.insert(CursorTheme::SplitHCursor, "split_h");
    cursorShapes.insert(CursorTheme::SplitHCursor, "col-resize");
    cursorShapes.insert(CursorTheme::SplitHCursor, "sb_h_double_arrow");
    cursorShapes.insert(CursorTheme::SplitHCursor, "043a9f68147c53184671403ffa811cc5");
    cursorShapes.insert(CursorTheme::SplitHCursor, "14fef782d02440884392942c11205230");

    cursorShapes.insert(CursorTheme::PointingHandCursor, "pointing_hand");
    cursorShapes.insert(CursorTheme::PointingHandCursor, "pointer");
    cursorShapes.insert(CursorTheme::PointingHandCursor, "hand1");
    cursorShapes.insert(CursorTheme::PointingHandCursor, "e29285e634086352946a0e7090d73106");

    cursorShapes.insert(CursorTheme::ForbiddenCursor, "forbidden");
    cursorShapes.insert(CursorTheme::ForbiddenCursor, "not-allowed");
    cursorShapes.insert(CursorTheme::ForbiddenCursor, "crossed_circle");
    cursorShapes.insert(CursorTheme::ForbiddenCursor, "circle");
    cursorShapes.insert(CursorTheme::ForbiddenCursor, "03b6e0fcb3499374a867c041f52298f0");

    cursorShapes.insert(CursorTheme::WhatsThisCursor, "whats_this");
    cursorShapes.insert(CursorTheme::WhatsThisCursor, "help");
    cursorShapes.insert(CursorTheme::WhatsThisCursor, "question_arrow");
    cursorShapes.insert(CursorTheme::WhatsThisCursor, "5c6cd98b3f3ebcb1f9c7f1c204630408");
    cursorShapes.insert(CursorTheme::WhatsThisCursor, "d9ce0ab605698f320427677b458ad60b");

    cursorShapes.insert(CursorTheme::BusyCursor, "left_ptr_watch");
    cursorShapes.insert(CursorTheme::BusyCursor, "half-busy");
    cursorShapes.insert(CursorTheme::BusyCursor, "progress");
    cursorShapes.insert(CursorTheme::BusyCursor, "00000000000000020006000e7e9ffc3f");
    cursorShapes.insert(CursorTheme::BusyCursor, "08e8e1c95fe2fc01f976f1e063a24ccd");

    cursorShapes.insert(CursorTheme::OpenHandCursor, "openhand");
    cursorShapes.insert(CursorTheme::OpenHandCursor, "fleur");
    cursorShapes.insert(CursorTheme::OpenHandCursor, "5aca4d189052212118709018842178c0");
    cursorShapes.insert(CursorTheme::OpenHandCursor, "9d800788f1b08800ae810202380a0822");

    cursorShapes.insert(CursorTheme::ClosedHandCursor, "closedhand");
    cursorShapes.insert(CursorTheme::ClosedHandCursor, "grabbing");
    cursorShapes.insert(CursorTheme::ClosedHandCursor, "208530c400c041818281048008011002");

    cursorShapes.insert(CursorTheme::DragCopyCursor, "dnd-copy");
    cursorShapes.insert(CursorTheme::DragCopyCursor, "copy");

    cursorShapes.insert(CursorTheme::DragMoveCursor, "dnd-move");
    cursorShapes.insert(CursorTheme::DragMoveCursor, "move");

    cursorShapes.insert(CursorTheme::DragLinkCursor, "dnd-link");
    cursorShapes.insert(CursorTheme::DragLinkCursor, "link");

    cursorShapes.insert(CursorTheme::ResizeNorthCursor, "n-resize");
    cursorShapes.insert(CursorTheme::ResizeNorthCursor, "top_side");

    cursorShapes.insert(CursorTheme::ResizeSouthCursor, "s-resize");
    cursorShapes.insert(CursorTheme::ResizeSouthCursor, "bottom_side");

    cursorShapes.insert(CursorTheme::ResizeEastCursor, "e-resize");
    cursorShapes.insert(CursorTheme::ResizeEastCursor, "right_side");

    cursorShapes.insert(CursorTheme::ResizeWestCursor, "w-resize");
    cursorShapes.insert(CursorTheme::ResizeWestCursor, "left_side");

    cursorShapes.insert(CursorTheme::ResizeNorthWestCursor, "nw-resize");
    cursorShapes.insert(CursorTheme::ResizeNorthWestCursor, "top_left_corner");

    cursorShapes.insert(CursorTheme::ResizeSouthEastCursor, "se-resize");
    cursorShapes.insert(CursorTheme::ResizeSouthEastCursor, "bottom_right_corner");

    cursorShapes.insert(CursorTheme::ResizeNorthEastCursor, "ne-resize");
    cursorShapes.insert(CursorTheme::ResizeNorthEastCursor, "top_right_corner");

    cursorShapes.insert(CursorTheme::ResizeSouthWestCursor, "sw-resize");
    cursorShapes.insert(CursorTheme::ResizeSouthWestCursor, "bottom_left_corner");
}

void CursorThemePrivate::loadTheme()
{
    if (theme)
        return;

    theme = wl_cursor_theme_load(cursorThemeName.toUtf8().constData(),
                                 cursorSize, ShmPrivate::get(pool->shm())->object());
    if (theme)
        qCDebug(WLCURSORTHEME) << "Cursor theme" << cursorThemeName << "loaded";
    else
        qCWarning(WLCURSORTHEME) << "Unable to load cursor theme" << cursorThemeName;
}

void CursorThemePrivate::destroyTheme()
{
    if (!theme)
        return;

    wl_cursor_theme_destroy(theme);
    theme = Q_NULLPTR;
}

wl_cursor *CursorThemePrivate::requestCursor(CursorTheme::CursorShape shape)
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
    if (!cursor && shape != CursorTheme::ArrowCursor)
        cursor = requestCursor(CursorTheme::ArrowCursor);

    return cursor;
}

/*
 * CursorTheme
 */

CursorTheme::CursorTheme(Compositor *compositor, ShmPool *pool, Seat *seat)
    : QObject(*new CursorThemePrivate(), seat)
{
    d_func()->compositor = compositor;
    d_func()->pool = pool;
    d_func()->seat = seat;
}

wl_cursor_image *CursorTheme::cursorImage(CursorShape shape)
{
    Q_D(CursorTheme);

    // Try to load the theme
    d->loadTheme();
    if (!d->theme)
        return Q_NULLPTR;

    wl_cursor *cursor = d->requestCursor(shape);
    if (!cursor || cursor->image_count <= 0)
        return Q_NULLPTR;
    return cursor->images[0];
}

void CursorTheme::changeCursor(CursorShape shape)
{
    Q_D(CursorTheme);

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
    if (d->seat->pointer()) {
        wl_buffer *wlBuffer = wl_cursor_image_get_buffer(image);
        if (!wlBuffer)
            return;
        Buffer *buffer = BufferPrivate::fromWlBuffer(wlBuffer);

        if (!d->cursorSurface)
            d->cursorSurface = d->compositor->createSurface(this);
        d->seat->pointer()->setCursor(d->cursorSurface, QPoint(image->hotspot_x, image->hotspot_y));
        d->cursorSurface->attach(buffer, QPoint(0, 0));
        d->cursorSurface->damage(QRect(0, 0, image->width, image->height));
        d->cursorSurface->commit(Surface::NoCommitMode);
    }
}

} // namespace Client

} // namespace GreenIsland

#include "moc_cursortheme.cpp"
