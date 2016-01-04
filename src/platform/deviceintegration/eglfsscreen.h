/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#ifndef GREENISLAND_EGLFSSCREEN_H
#define GREENISLAND_EGLFSSCREEN_H

#include <QtGui/qpa/qplatformscreen.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <EGL/egl.h>

class QOpenGLContext;

namespace GreenIsland {

namespace Platform {

class EglFSWindow;

class GREENISLANDPLATFORM_EXPORT EglFSScreen : public QPlatformScreen
{
public:
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    enum PowerState {
        PowerStateOn,
        PowerStateStandby,
        PowerStateSuspend,
        PowerStateOff
    };
#endif

    EglFSScreen(EGLDisplay display);
    ~EglFSScreen();

    QRect geometry() const Q_DECL_OVERRIDE;
    int depth() const Q_DECL_OVERRIDE;
    QImage::Format format() const Q_DECL_OVERRIDE;

    QSizeF physicalSize() const Q_DECL_OVERRIDE;
    QDpi logicalDpi() const Q_DECL_OVERRIDE;
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    qreal pixelDensity() const Q_DECL_OVERRIDE;
#endif
    Qt::ScreenOrientation nativeOrientation() const Q_DECL_OVERRIDE;
    Qt::ScreenOrientation orientation() const Q_DECL_OVERRIDE;

    QPlatformCursor *cursor() const Q_DECL_OVERRIDE;

    qreal refreshRate() const Q_DECL_OVERRIDE;

    QPixmap grabWindow(WId wid, int x, int y, int width, int height) const Q_DECL_OVERRIDE;

    EGLSurface primarySurface() const { return m_surface; }

    EGLDisplay display() const { return m_dpy; }

    void handleCursorMove(const QPoint &pos);

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    virtual PowerState powerState() const;
    virtual void setPowerState(PowerState state);
#endif

private:
    void setPrimarySurface(EGLSurface surface);

    EGLDisplay m_dpy;
    QWindow *m_pointerWindow;
    EGLSurface m_surface;
    QPlatformCursor *m_cursor;

    friend class EglFSWindow;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSSCREEN_H
