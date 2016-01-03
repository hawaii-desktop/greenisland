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

#ifndef GREENISLAND_EGLFSWINDOW_H
#define GREENISLAND_EGLFSWINDOW_H

#include <QtGui/qpa/qplatformwindow.h>

#include <GreenIsland/Platform/OpenGLCompositor>

#include "eglfsscreen.h"

#include <EGL/egl.h>

class QPlatformTextureList;

namespace GreenIsland {

namespace Platform {

class OpenGLCompositorBackingStore;

class GREENISLANDPLATFORM_EXPORT EglFSWindow : public QPlatformWindow, public OpenGLCompositorWindow
{
public:
    EglFSWindow(QWindow *w);
    ~EglFSWindow();

    void create();
    void destroy();

    void setGeometry(const QRect &) Q_DECL_OVERRIDE;
    QRect geometry() const Q_DECL_OVERRIDE;
    void setVisible(bool visible) Q_DECL_OVERRIDE;
    void requestActivateWindow() Q_DECL_OVERRIDE;
    void raise() Q_DECL_OVERRIDE;
    void lower() Q_DECL_OVERRIDE;

    void propagateSizeHints() Q_DECL_OVERRIDE { }
    void setMask(const QRegion &) Q_DECL_OVERRIDE { }
    bool setKeyboardGrabEnabled(bool) Q_DECL_OVERRIDE { return false; }
    bool setMouseGrabEnabled(bool) Q_DECL_OVERRIDE { return false; }
    void setOpacity(qreal) Q_DECL_OVERRIDE;
    WId winId() const Q_DECL_OVERRIDE;

    QSurfaceFormat format() const Q_DECL_OVERRIDE;

    EGLNativeWindowType eglWindow() const;
    EGLSurface surface() const;
    EglFSScreen *screen() const;

    bool hasNativeWindow() const { return m_flags.testFlag(HasNativeWindow); }

    virtual void invalidateSurface() Q_DECL_OVERRIDE;
    virtual void resetSurface();

    OpenGLCompositorBackingStore *backingStore() { return m_backingStore; }
    void setBackingStore(OpenGLCompositorBackingStore *backingStore) { m_backingStore = backingStore; }
    bool isRaster() const;

    QWindow *sourceWindow() const Q_DECL_OVERRIDE;
    const QPlatformTextureList *textures() const Q_DECL_OVERRIDE;
    void endCompositing() Q_DECL_OVERRIDE;

private:
    OpenGLCompositorBackingStore *m_backingStore;
    bool m_raster;
    WId m_winId;

    EGLSurface m_surface;
    EGLNativeWindowType m_window;

    EGLConfig m_config;
    QSurfaceFormat m_format;

    enum Flag {
        Created = 0x01,
        HasNativeWindow = 0x02
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Flags m_flags;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSWINDOW_H
