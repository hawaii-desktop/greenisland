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

#ifndef GREENISLAND_CLIENT_WLCURSORTHEME_H
#define GREENISLAND_CLIENT_WLCURSORTHEME_H

#include <QtCore/QLoggingCategory>
#include <QtGui/QCursor>

Q_DECLARE_LOGGING_CATEGORY(WLCURSORTHEME)

struct wl_cursor_image;

namespace GreenIsland {

namespace Client {

class WlCursorThemePrivate;
class WlSeat;
class WlShmPool;

class WlCursorTheme
{
public:
    enum CursorShape {
        ArrowCursor = Qt::ArrowCursor,
        UpArrowCursor,
        CrossCursor,
        WaitCursor,
        IBeamCursor,
        SizeVerCursor,
        SizeHorCursor,
        SizeBDiagCursor,
        SizeFDiagCursor,
        SizeAllCursor,
        BlankCursor,
        SplitVCursor,
        SplitHCursor,
        PointingHandCursor,
        ForbiddenCursor,
        WhatsThisCursor,
        BusyCursor,
        OpenHandCursor,
        ClosedHandCursor,
        DragCopyCursor,
        DragMoveCursor,
        DragLinkCursor,
        ResizeNorthCursor = Qt::CustomCursor + 1,
        ResizeSouthCursor,
        ResizeEastCursor,
        ResizeWestCursor,
        ResizeNorthWestCursor,
        ResizeSouthEastCursor,
        ResizeNorthEastCursor,
        ResizeSouthWestCursor
    };

    WlCursorTheme(WlShmPool *pool, WlSeat *seat);
    ~WlCursorTheme();

    wl_cursor_image *cursorImage(CursorShape shape);

    void changeCursor(CursorShape shape);

private:
    WlCursorThemePrivate *const d;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLAND_CLIENT_WLCURSORTHEME_H
