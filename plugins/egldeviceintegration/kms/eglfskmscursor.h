/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_EGLFSKMSCURSOR_H
#define GREENISLAND_EGLFSKMSCURSOR_H

#include <QtCore/QList>
#include <QtGui/QImage>
#include <QtGui/qpa/qplatformcursor.h>

#include <gbm.h>

namespace GreenIsland {

namespace Platform {

class EglFSKmsScreen;

class EglFSKmsCursor : public QPlatformCursor
{
    Q_OBJECT
public:
    EglFSKmsCursor(EglFSKmsScreen *screen);
    ~EglFSKmsCursor();

    // input methods
    void pointerEvent(const QMouseEvent & event) Q_DECL_OVERRIDE;
#ifndef QT_NO_CURSOR
    void changeCursor(QCursor * windowCursor, QWindow * window) Q_DECL_OVERRIDE;
#endif
    QPoint pos() const Q_DECL_OVERRIDE;
    void setPos(const QPoint &pos) Q_DECL_OVERRIDE;

private:
    void initCursorAtlas();

    EglFSKmsScreen *m_screen;
    QSize m_cursorSize;
    gbm_bo *m_bo;
    QPoint m_pos;
    QPlatformCursorImage m_cursorImage;
    bool m_visible;

    // cursor atlas information
    struct CursorAtlas {
        CursorAtlas() : cursorsPerRow(0), cursorWidth(0), cursorHeight(0) { }
        int cursorsPerRow;
        int width, height; // width and height of the atlas
        int cursorWidth, cursorHeight; // width and height of cursors inside the atlas
        QList<QPoint> hotSpots;
        QImage image;
    } m_cursorAtlas;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSKMSCURSOR_H
