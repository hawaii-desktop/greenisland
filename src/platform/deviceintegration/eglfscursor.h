/****************************************************************************
 * This file is part of Hawaii.
 *
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

#ifndef GREENISLAND_EGLFSCURSOR_H
#define GREENISLAND_EGLFSCURSOR_H

#include <QtGui/QOpenGLFunctions>
#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/qpa/qplatformscreen.h>
#include <QtGui/private/qinputdevicemanager_p.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

class QOpenGLShaderProgram;

namespace GreenIsland {

namespace Platform {

class EglFSCursor;
class EglFSScreen;

class EglFSCursorDeviceListener : public QObject
{
    Q_OBJECT
public:
    EglFSCursorDeviceListener(EglFSCursor *cursor) : m_cursor(cursor) { }
    bool hasMouse() const;

public slots:
    void onDeviceListChanged(QInputDeviceManager::DeviceType type);

private:
    EglFSCursor *m_cursor;
};

class GREENISLANDPLATFORM_EXPORT EglFSCursor : public QPlatformCursor, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    EglFSCursor(QPlatformScreen *screen);
    ~EglFSCursor();

#ifndef QT_NO_CURSOR
    void changeCursor(QCursor *cursor, QWindow *widget) Q_DECL_OVERRIDE;
#endif
    void pointerEvent(const QMouseEvent &event) Q_DECL_OVERRIDE;
    QPoint pos() const Q_DECL_OVERRIDE;
    void setPos(const QPoint &pos) Q_DECL_OVERRIDE;

    QRect cursorRect() const;
    void paintOnScreen();
    void resetResources();

    void updateMouseStatus();

private:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
#ifndef QT_NO_CURSOR
    bool setCurrentCursor(QCursor *cursor);
#endif
    void draw(const QRectF &rect);
    void update(const QRegion &region);
    void createShaderPrograms();
    void createCursorTexture(uint *texture, const QImage &image);
    void initCursorAtlas();

    // current cursor information
    struct Cursor {
        Cursor() : texture(0), shape(Qt::BlankCursor), customCursorTexture(0), customCursorPending(false) { }
        uint texture; // a texture from 'image' or the atlas
        Qt::CursorShape shape;
        QRectF textureRect; // normalized rect inside texture
        QSize size; // size of the cursor
        QPoint hotSpot;
        QImage customCursorImage;
        QPoint pos; // current cursor position
        uint customCursorTexture;
        bool customCursorPending;
    } m_cursor;

    // cursor atlas information
    struct CursorAtlas {
        CursorAtlas() : cursorsPerRow(0), texture(0), cursorWidth(0), cursorHeight(0) { }
        int cursorsPerRow;
        uint texture;
        int width, height; // width and height of the atlas
        int cursorWidth, cursorHeight; // width and height of cursors inside the atlas
        QList<QPoint> hotSpots;
        QImage image; // valid until it's uploaded
    } m_cursorAtlas;

    bool m_visible;
    EglFSScreen *m_screen;
    QOpenGLShaderProgram *m_program;
    int m_textureEntry;
    EglFSCursorDeviceListener *m_deviceListener;
    bool m_updateRequested;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSCURSOR_H
