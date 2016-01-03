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

#include <QtGui/QSurface>

#include "logging.h"
#include "eglconvenience/eglconvenience.h"
#include "eglconvenience/eglpbuffer.h"
#include "deviceintegration/egldeviceintegration.h"
#include "deviceintegration/eglfscontext.h"
#include "deviceintegration/eglfswindow.h"
#include "deviceintegration/eglfscursor.h"

namespace GreenIsland {

namespace Platform {

EglFSContext::EglFSContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display,
                           EGLConfig *config, const QVariant &nativeHandle)
    : EGLPlatformContext(format, share, display, config, nativeHandle),
      m_tempWindow(0)
{
}

EGLSurface EglFSContext::eglSurfaceForPlatformSurface(QPlatformSurface *surface)
{
    if (surface->surface()->surfaceClass() == QSurface::Window)
        return static_cast<EglFSWindow *>(surface)->surface();
    else
        return static_cast<EGLPbuffer *>(surface)->pbuffer();
}

EGLSurface EglFSContext::createTemporaryOffscreenSurface()
{
    if (egl_device_integration()->supportsPBuffers())
        return EGLPlatformContext::createTemporaryOffscreenSurface();

    if (!m_tempWindow) {
        m_tempWindow = egl_device_integration()->createNativeOffscreenWindow(format());
        if (!m_tempWindow) {
            qCWarning(lcDeviceIntegration, "EglFSContext: Failed to create temporary native window");
            return EGL_NO_SURFACE;
        }
    }
    EGLConfig config = EglUtils::configFromGLFormat(eglDisplay(), format());
    return eglCreateWindowSurface(eglDisplay(), config, m_tempWindow, 0);
}

void EglFSContext::destroyTemporaryOffscreenSurface(EGLSurface surface)
{
    if (egl_device_integration()->supportsPBuffers()) {
        EGLPlatformContext::destroyTemporaryOffscreenSurface(surface);
    } else {
        eglDestroySurface(eglDisplay(), surface);
        egl_device_integration()->destroyNativeWindow(m_tempWindow);
        m_tempWindow = 0;
    }
}

void EglFSContext::swapBuffers(QPlatformSurface *surface)
{
    // draw the cursor
    if (surface->surface()->surfaceClass() == QSurface::Window) {
        QPlatformWindow *window = static_cast<QPlatformWindow *>(surface);
        if (EglFSCursor *cursor = qobject_cast<EglFSCursor *>(window->screen()->cursor()))
            cursor->paintOnScreen();
    }

    egl_device_integration()->waitForVSync(surface);
    EGLPlatformContext::swapBuffers(surface);
    egl_device_integration()->presentBuffer(surface);
}

} // namespace Platform

} // namespace GreenIsland
