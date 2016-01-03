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

#include <QtGui/QOffscreenSurface>

#include "eglpbuffer.h"
#include "eglconvenience.h"

namespace GreenIsland {

namespace Platform {

/*!
    \class QEGLPbuffer
    \brief A pbuffer-based implementation of QPlatformOffscreenSurface for EGL.
    \since 5.2
    \internal
    \ingroup qpa

    To use this implementation in the platform plugin simply
    reimplement QPlatformIntegration::createPlatformOffscreenSurface()
    and return a new instance of this class.
*/

EGLPbuffer::EGLPbuffer(EGLDisplay display, const QSurfaceFormat &format, QOffscreenSurface *offscreenSurface,
                         EGLPlatformContext::Flags flags)
    : QPlatformOffscreenSurface(offscreenSurface)
    , m_format(format)
    , m_display(display)
    , m_pbuffer(EGL_NO_SURFACE)
{
    bool hasSurfaceless = !flags.testFlag(EGLPlatformContext::NoSurfaceless)
            && EglUtils::hasEglExtension(display, "EGL_KHR_surfaceless_context");

    // Disable surfaceless contexts on Mesa for now. As of 10.6.0 and Intel at least, some
    // operations (glReadPixels) are unable to work without a surface since they at some
    // point temporarily unbind the current FBO and then later blow up in some seemingly
    // safe operations, like setting the viewport, that apparently need access to the
    // read/draw surface in the Intel backend.
    const char *vendor = eglQueryString(display, EGL_VENDOR); // hard to check for GL_ strings here, so blacklist all Mesa
    if (vendor && strstr(vendor, "Mesa"))
        hasSurfaceless = false;

    if (hasSurfaceless)
        return;

    EGLConfig config = EglUtils::configFromGLFormat(m_display, m_format, false, EGL_PBUFFER_BIT);

    if (config) {
        const EGLint attributes[] = {
            EGL_WIDTH, offscreenSurface->size().width(),
            EGL_HEIGHT, offscreenSurface->size().height(),
            EGL_LARGEST_PBUFFER, EGL_FALSE,
            EGL_NONE
        };

        m_pbuffer = eglCreatePbufferSurface(m_display, config, attributes);

        if (m_pbuffer != EGL_NO_SURFACE)
            m_format = EglUtils::glFormatFromConfig(m_display, config);
    }
}

EGLPbuffer::~EGLPbuffer()
{
    if (m_pbuffer != EGL_NO_SURFACE)
        eglDestroySurface(m_display, m_pbuffer);
}

} // namespace Platform

} // namespace GreenIsland
