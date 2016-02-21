/****************************************************************************
**
** Copyright (C) 2015-2016 Pier Luigi Fiorini
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
