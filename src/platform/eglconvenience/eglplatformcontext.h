/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 The Qt Company Ltd.
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QFunctionPointer getProcAddress(const char *procName) Q_DECL_OVERRIDE;
#else
    QFunctionPointer getProcAddress(const QByteArray &procName) Q_DECL_OVERRIDE;
#endif

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
    virtual void runGLchecks();

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
