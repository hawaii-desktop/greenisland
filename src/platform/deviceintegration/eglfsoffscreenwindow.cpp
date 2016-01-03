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

#include "logging.h"
#include "eglconvenience/eglconvenience.h"
#include "deviceintegration/egldeviceintegration.h"
#include "deviceintegration/eglfsoffscreenwindow.h"

namespace GreenIsland {

namespace Platform {

/*
    In some cases pbuffers are not available. Triggering QtGui's built-in
    fallback for a hidden QWindow is not suitable for eglfs since this would be
    treated as an attempt to create multiple top-level, native windows.

    Therefore this class is provided as an alternative to EGLPbuffer.

    This class requires the hooks to implement createNativeOffscreenWindow().
*/

EglFSOffscreenWindow::EglFSOffscreenWindow(EGLDisplay display, const QSurfaceFormat &format, QOffscreenSurface *offscreenSurface)
    : QPlatformOffscreenSurface(offscreenSurface)
    , m_format(format)
    , m_display(display)
    , m_surface(EGL_NO_SURFACE)
    , m_window(0)
{
    m_window = egl_device_integration()->createNativeOffscreenWindow(format);
    if (!m_window) {
        qCWarning(lcDeviceIntegration, "EglFSOffscreenWindow: Failed to create native window");
        return;
    }
    EGLConfig config = EglUtils::configFromGLFormat(m_display, m_format);
    m_surface = eglCreateWindowSurface(m_display, config, m_window, 0);
    if (m_surface != EGL_NO_SURFACE)
        m_format = EglUtils::glFormatFromConfig(m_display, config);
}

EglFSOffscreenWindow::~EglFSOffscreenWindow()
{
    if (m_surface != EGL_NO_SURFACE)
        eglDestroySurface(m_display, m_surface);
    if (m_window)
        egl_device_integration()->destroyNativeWindow(m_window);
}

} // namespace Platform

} // namespace GreenIsland
