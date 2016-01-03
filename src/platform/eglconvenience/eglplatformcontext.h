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

#ifndef GREENISLAND_EGLPLATFORMCONTEXT_H
#define GREENISLAND_EGLPLATFORMCONTEXT_H

#include <QtCore/qtextstream.h>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/qpa/qplatformopenglcontext.h>
#include <QtCore/QVariant>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <EGL/egl.h>

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT EGLPlatformContext : public QPlatformOpenGLContext
{
public:
    enum Flag {
        NoSurfaceless = 0x01
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    EGLPlatformContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display,
                        EGLConfig *config = 0, const QVariant &nativeHandle = QVariant(),
                        Flags flags = 0);
    ~EGLPlatformContext();

    void initialize() Q_DECL_OVERRIDE;
    bool makeCurrent(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    void doneCurrent() Q_DECL_OVERRIDE;
    void swapBuffers(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    QFunctionPointer getProcAddress(const QByteArray &procName) Q_DECL_OVERRIDE;

    QSurfaceFormat format() const Q_DECL_OVERRIDE;
    bool isSharing() const Q_DECL_OVERRIDE { return m_shareContext != EGL_NO_CONTEXT; }
    bool isValid() const Q_DECL_OVERRIDE { return m_eglContext != EGL_NO_CONTEXT; }

    EGLContext eglContext() const;
    EGLDisplay eglDisplay() const;
    EGLConfig eglConfig() const;

protected:
    virtual EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) = 0;
    virtual EGLSurface createTemporaryOffscreenSurface();
    virtual void destroyTemporaryOffscreenSurface(EGLSurface surface);

private:
    void init(const QSurfaceFormat &format, QPlatformOpenGLContext *share);
    void adopt(const QVariant &nativeHandle, QPlatformOpenGLContext *share);
    void updateFormatFromGL();

    EGLContext m_eglContext;
    EGLContext m_shareContext;
    EGLDisplay m_eglDisplay;
    EGLConfig m_eglConfig;
    QSurfaceFormat m_format;
    EGLenum m_api;
    int m_swapInterval;
    bool m_swapIntervalEnvChecked;
    int m_swapIntervalFromEnv;
    Flags m_flags;
    bool m_ownsContext;
    QVector<EGLint> m_contextAttrs;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(EGLPlatformContext::Flags)

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLPLATFORMCONTEXT_H
