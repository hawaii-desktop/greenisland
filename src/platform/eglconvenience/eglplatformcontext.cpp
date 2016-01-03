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

#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/QOpenGLContext>
#include <QtPlatformHeaders/QEGLNativeContext>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qjnihelpers_p.h>
#endif

#include "logging.h"
#include "eglplatformcontext.h"
#include "eglconvenience.h"
#include "eglpbuffer.h"

namespace GreenIsland {

namespace Platform {

/*!
    \class QEGLPlatformContext
    \brief An EGL context implementation.
    \since 5.2
    \internal
    \ingroup qpa

    Implement QPlatformOpenGLContext using EGL. To use it in platform
    plugins a subclass must be created since
    eglSurfaceForPlatformSurface() has to be reimplemented. This
    function is used for mapping platform surfaces (windows) to EGL
    surfaces and is necessary since different platform plugins may
    have different ways of handling native windows (for example, a
    plugin may choose not to back every platform window by a real EGL
    surface). Other than that, no further customization is necessary.
 */

// Constants from EGL_KHR_create_context
#ifndef EGL_CONTEXT_MINOR_VERSION_KHR
#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30FB
#endif
#ifndef EGL_CONTEXT_FLAGS_KHR
#define EGL_CONTEXT_FLAGS_KHR 0x30FC
#endif
#ifndef EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR 0x30FD
#endif
#ifndef EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR 0x00000001
#endif
#ifndef EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR 0x00000002
#endif
#ifndef EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR 0x00000001
#endif
#ifndef EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR 0x00000002
#endif

// Constants for OpenGL which are not available in the ES headers.
#ifndef GL_CONTEXT_FLAGS
#define GL_CONTEXT_FLAGS 0x821E
#endif
#ifndef GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#endif
#ifndef GL_CONTEXT_FLAG_DEBUG_BIT
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#endif
#ifndef GL_CONTEXT_PROFILE_MASK
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif
#ifndef GL_CONTEXT_CORE_PROFILE_BIT
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif
#ifndef GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

EGLPlatformContext::EGLPlatformContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display,
                                         EGLConfig *config, const QVariant &nativeHandle, Flags flags)
    : m_eglDisplay(display)
    , m_swapInterval(-1)
    , m_swapIntervalEnvChecked(false)
    , m_swapIntervalFromEnv(-1)
    , m_flags(flags)
{
    if (nativeHandle.isNull()) {
        m_eglConfig = config ? *config : EglUtils::configFromGLFormat(display, format);
        m_ownsContext = true;
        init(format, share);
    } else {
        m_ownsContext = false;
        adopt(nativeHandle, share);
    }
}

void EGLPlatformContext::init(const QSurfaceFormat &format, QPlatformOpenGLContext *share)
{
    m_format = EglUtils::glFormatFromConfig(m_eglDisplay, m_eglConfig);
    // m_format now has the renderableType() resolved (it cannot be Default anymore)
    // but does not yet contain version, profile, options.
    m_shareContext = share ? static_cast<EGLPlatformContext *>(share)->m_eglContext : 0;

    QVector<EGLint> contextAttrs;
    contextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    contextAttrs.append(format.majorVersion());
    const bool hasKHRCreateContext = EglUtils::hasEglExtension(m_eglDisplay, "EGL_KHR_create_context");
    if (hasKHRCreateContext) {
        contextAttrs.append(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttrs.append(format.minorVersion());
        int flags = 0;
        // The debug bit is supported both for OpenGL and OpenGL ES.
        if (format.testOption(QSurfaceFormat::DebugContext))
            flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
        // The fwdcompat bit is only for OpenGL 3.0+.
        if (m_format.renderableType() == QSurfaceFormat::OpenGL
                && format.majorVersion() >= 3
                && !format.testOption(QSurfaceFormat::DeprecatedFunctions))
            flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;
        if (flags) {
            contextAttrs.append(EGL_CONTEXT_FLAGS_KHR);
            contextAttrs.append(flags);
        }
        // Profiles are OpenGL only and mandatory in 3.2+. The value is silently ignored for < 3.2.
        if (m_format.renderableType() == QSurfaceFormat::OpenGL) {
            contextAttrs.append(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR);
            contextAttrs.append(format.profile() == QSurfaceFormat::CoreProfile
                                ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
                                : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR);
        }
    }
    contextAttrs.append(EGL_NONE);
    m_contextAttrs = contextAttrs;

    switch (m_format.renderableType()) {
    case QSurfaceFormat::OpenVG:
        m_api = EGL_OPENVG_API;
        break;
#ifdef EGL_VERSION_1_4
    case QSurfaceFormat::OpenGL:
        m_api = EGL_OPENGL_API;
        break;
#endif // EGL_VERSION_1_4
    default:
        m_api = EGL_OPENGL_ES_API;
        break;
    }

    eglBindAPI(m_api);
    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, m_shareContext, contextAttrs.constData());
    if (m_eglContext == EGL_NO_CONTEXT && m_shareContext != EGL_NO_CONTEXT) {
        m_shareContext = 0;
        m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, 0, contextAttrs.constData());
    }

    if (m_eglContext == EGL_NO_CONTEXT) {
        qCWarning(lcEglConvenience, "Failed to create context: %x", eglGetError());
        return;
    }

    static const bool printConfig = qEnvironmentVariableIntValue("GREENISLAND_QPA_DEBUG");
    if (printConfig) {
        qDebug() << "Created context for format" << format << "with config:";
        EglUtils::printEglConfig(m_eglDisplay, m_eglConfig);
    }

    // Cannot just call updateFormatFromGL() since it relies on virtuals. Defer it to initialize().
}

void EGLPlatformContext::adopt(const QVariant &nativeHandle, QPlatformOpenGLContext *share)
{
    if (!nativeHandle.canConvert<QEGLNativeContext>()) {
        qCWarning(lcEglConvenience, "Requires a QEGLNativeContext");
        return;
    }
    QEGLNativeContext handle = nativeHandle.value<QEGLNativeContext>();
    EGLContext context = handle.context();
    if (!context) {
        qCWarning(lcEglConvenience, "No EGLContext given");
        return;
    }

    // A context belonging to a given EGLDisplay cannot be used with another one.
    if (handle.display() != m_eglDisplay) {
        qCWarning(lcEglConvenience, "Cannot adopt context from different display");
        return;
    }

    // Figure out the EGLConfig.
    EGLint value = 0;
    eglQueryContext(m_eglDisplay, context, EGL_CONFIG_ID, &value);
    EGLint n = 0;
    EGLConfig cfg;
    const EGLint attribs[] = { EGL_CONFIG_ID, value, EGL_NONE };
    if (eglChooseConfig(m_eglDisplay, attribs, &cfg, 1, &n) && n == 1) {
        m_eglConfig = cfg;
        m_format = EglUtils::glFormatFromConfig(m_eglDisplay, m_eglConfig);
    } else {
        qCWarning(lcEglConvenience, "Failed to get framebuffer configuration for context");
    }

    // Fetch client API type.
    value = 0;
    eglQueryContext(m_eglDisplay, context, EGL_CONTEXT_CLIENT_TYPE, &value);
    if (value == EGL_OPENGL_API || value == EGL_OPENGL_ES_API) {
        m_api = value;
        eglBindAPI(m_api);
    } else {
        qCWarning(lcEglConvenience, "Failed to get client API type");
        m_api = EGL_OPENGL_ES_API;
    }

    m_eglContext = context;
    m_shareContext = share ? static_cast<EGLPlatformContext *>(share)->m_eglContext : 0;
    updateFormatFromGL();
}

void EGLPlatformContext::initialize()
{
    if (m_eglContext != EGL_NO_CONTEXT)
        updateFormatFromGL();
}

// Base implementation for pbuffers. Subclasses will handle the specialized cases for
// platforms without pbuffers.
EGLSurface EGLPlatformContext::createTemporaryOffscreenSurface()
{
    // Make the context current to ensure the GL version query works. This needs a surface too.
    const EGLint pbufferAttributes[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_LARGEST_PBUFFER, EGL_FALSE,
        EGL_NONE
    };

    // Cannot just pass m_eglConfig because it may not be suitable for pbuffers. Instead,
    // do what QEGLPbuffer would do: request a config with the same attributes but with
    // PBUFFER_BIT set.
    EGLConfig config = EglUtils::configFromGLFormat(m_eglDisplay, m_format, false, EGL_PBUFFER_BIT);

    return eglCreatePbufferSurface(m_eglDisplay, config, pbufferAttributes);
}

void EGLPlatformContext::destroyTemporaryOffscreenSurface(EGLSurface surface)
{
    eglDestroySurface(m_eglDisplay, surface);
}

void EGLPlatformContext::updateFormatFromGL()
{
#ifndef QT_NO_OPENGL
    // Have to save & restore to prevent QOpenGLContext::currentContext() from becoming
    // inconsistent after QOpenGLContext::create().
    EGLDisplay prevDisplay = eglGetCurrentDisplay();
    if (prevDisplay == EGL_NO_DISPLAY) // when no context is current
        prevDisplay = m_eglDisplay;
    EGLContext prevContext = eglGetCurrentContext();
    EGLSurface prevSurfaceDraw = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface prevSurfaceRead = eglGetCurrentSurface(EGL_READ);

    // Rely on the surfaceless extension, if available. This is beneficial since we can
    // avoid creating an extra pbuffer surface which is apparently troublesome with some
    // drivers (Mesa) when certain attributes are present (multisampling).
    EGLSurface tempSurface = EGL_NO_SURFACE;
    EGLContext tempContext = EGL_NO_CONTEXT;
    if (m_flags.testFlag(NoSurfaceless) || !EglUtils::hasEglExtension(m_eglDisplay, "EGL_KHR_surfaceless_context"))
        tempSurface = createTemporaryOffscreenSurface();

    EGLBoolean ok = eglMakeCurrent(m_eglDisplay, tempSurface, tempSurface, m_eglContext);
    if (!ok) {
        EGLConfig config = EglUtils::configFromGLFormat(m_eglDisplay, m_format, false, EGL_PBUFFER_BIT);
        tempContext = eglCreateContext(m_eglDisplay, config, 0, m_contextAttrs.constData());
        if (tempContext != EGL_NO_CONTEXT)
            ok = eglMakeCurrent(m_eglDisplay, tempSurface, tempSurface, tempContext);
    }
    if (ok) {
        if (m_format.renderableType() == QSurfaceFormat::OpenGL
                || m_format.renderableType() == QSurfaceFormat::OpenGLES) {
            const GLubyte *s = glGetString(GL_VERSION);
            if (s) {
                QByteArray version = QByteArray(reinterpret_cast<const char *>(s));
                int major, minor;
                if (QPlatformOpenGLContext::parseOpenGLVersion(version, major, minor)) {
#ifdef Q_OS_ANDROID
                    // Some Android 4.2.2 devices report OpenGL ES 3.0 without the functions being available.
                    static int apiLevel = QtAndroidPrivate::androidSdkVersion();
                    if (apiLevel <= 17 && major >= 3) {
                        major = 2;
                        minor = 0;
                    }
#endif
                    m_format.setMajorVersion(major);
                    m_format.setMinorVersion(minor);
                }
            }
            m_format.setProfile(QSurfaceFormat::NoProfile);
            m_format.setOptions(QSurfaceFormat::FormatOptions());
            if (m_format.renderableType() == QSurfaceFormat::OpenGL) {
                // Check profile and options.
                if (m_format.majorVersion() < 3) {
                    m_format.setOption(QSurfaceFormat::DeprecatedFunctions);
                } else {
                    GLint value = 0;
                    glGetIntegerv(GL_CONTEXT_FLAGS, &value);
                    if (!(value & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT))
                        m_format.setOption(QSurfaceFormat::DeprecatedFunctions);
                    if (value & GL_CONTEXT_FLAG_DEBUG_BIT)
                        m_format.setOption(QSurfaceFormat::DebugContext);
                    if (m_format.version() >= qMakePair(3, 2)) {
                        value = 0;
                        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &value);
                        if (value & GL_CONTEXT_CORE_PROFILE_BIT)
                            m_format.setProfile(QSurfaceFormat::CoreProfile);
                        else if (value & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
                            m_format.setProfile(QSurfaceFormat::CompatibilityProfile);
                    }
                }
            }
        }
        eglMakeCurrent(prevDisplay, prevSurfaceDraw, prevSurfaceRead, prevContext);
    } else {
        qCWarning(lcEglConvenience, "Failed to make temporary surface current, format not updated (%x)", eglGetError());
    }
    if (tempSurface != EGL_NO_SURFACE)
        destroyTemporaryOffscreenSurface(tempSurface);
    if (tempContext != EGL_NO_CONTEXT)
        eglDestroyContext(m_eglDisplay, tempContext);
#endif // QT_NO_OPENGL
}

bool EGLPlatformContext::makeCurrent(QPlatformSurface *surface)
{
    Q_ASSERT(surface->surface()->supportsOpenGL());

    eglBindAPI(m_api);

    EGLSurface eglSurface = eglSurfaceForPlatformSurface(surface);

    // shortcut: on some GPUs, eglMakeCurrent is not a cheap operation
    if (eglGetCurrentContext() == m_eglContext &&
            eglGetCurrentDisplay() == m_eglDisplay &&
            eglGetCurrentSurface(EGL_READ) == eglSurface &&
            eglGetCurrentSurface(EGL_DRAW) == eglSurface) {
        return true;
    }

    const bool ok = eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_eglContext);
    if (ok) {
        if (!m_swapIntervalEnvChecked) {
            m_swapIntervalEnvChecked = true;
            if (qEnvironmentVariableIsSet("GREENISLAND_QPA_SWAPINTERVAL")) {
                QByteArray swapIntervalString = qgetenv("GREENISLAND_QPA_SWAPINTERVAL");
                bool intervalOk;
                const int swapInterval = swapIntervalString.toInt(&intervalOk);
                if (intervalOk)
                    m_swapIntervalFromEnv = swapInterval;
            }
        }
        const int requestedSwapInterval = m_swapIntervalFromEnv >= 0
                ? m_swapIntervalFromEnv
                : surface->format().swapInterval();
        if (requestedSwapInterval >= 0 && m_swapInterval != requestedSwapInterval) {
            m_swapInterval = requestedSwapInterval;
            if (eglSurface != EGL_NO_SURFACE) // skip if using surfaceless context
                eglSwapInterval(eglDisplay(), m_swapInterval);
        }
    } else {
        qCWarning(lcEglConvenience, "eglMakeCurrent failed: %x", eglGetError());
    }

    return ok;
}

EGLPlatformContext::~EGLPlatformContext()
{
    if (m_ownsContext && m_eglContext != EGL_NO_CONTEXT)
        eglDestroyContext(m_eglDisplay, m_eglContext);

    m_eglContext = EGL_NO_CONTEXT;
}

void EGLPlatformContext::doneCurrent()
{
    eglBindAPI(m_api);
    bool ok = eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (!ok)
        qCWarning(lcEglConvenience, "eglMakeCurrent failed: %x", eglGetError());
}

void EGLPlatformContext::swapBuffers(QPlatformSurface *surface)
{
    eglBindAPI(m_api);
    EGLSurface eglSurface = eglSurfaceForPlatformSurface(surface);
    if (eglSurface != EGL_NO_SURFACE) { // skip if using surfaceless context
        bool ok = eglSwapBuffers(m_eglDisplay, eglSurface);
        if (!ok)
            qCWarning(lcEglConvenience, "eglSwapBuffers failed: %x", eglGetError());
    }
}

void (*EGLPlatformContext::getProcAddress(const QByteArray &procName)) ()
{
    eglBindAPI(m_api);
    return eglGetProcAddress(procName.constData());
}

QSurfaceFormat EGLPlatformContext::format() const
{
    return m_format;
}

EGLContext EGLPlatformContext::eglContext() const
{
    return m_eglContext;
}

EGLDisplay EGLPlatformContext::eglDisplay() const
{
    return m_eglDisplay;
}

EGLConfig EGLPlatformContext::eglConfig() const
{
    return m_eglConfig;
}

} // namespace Platform

} // namespace GreenIsland
