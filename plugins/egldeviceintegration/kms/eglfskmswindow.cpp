/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#include <QtGui/qpa/qwindowsysteminterface.h>

#include <GreenIsland/Platform/EGLPlatformContext>

#include "eglfskmsscreen.h"
#include "eglfskmswindow.h"

namespace GreenIsland {

namespace Platform {

EglFSKmsWindow::EglFSKmsWindow(QWindow *w)
    : EglFSWindow(w)
{
}

void EglFSKmsWindow::resizeSurface()
{
    if (window()->type() == Qt::Desktop) {
        QRect fullscreenRect(QPoint(), screen()->availableGeometry().size());
        QPlatformWindow::setGeometry(fullscreenRect);
        QWindowSystemInterface::handleGeometryChange(window(), fullscreenRect);
        return;
    }

    EglFSKmsScreen *kmsScreen = static_cast<EglFSKmsScreen *>(screen());

    setGeometry(QRect());
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(0, 0), geometry().size()));

    EglFSScreen *nativeScreen = static_cast<EglFSScreen *>(screen());
    EGLDisplay display = nativeScreen->display();
    EGLNativeWindowType window = reinterpret_cast<EGLNativeWindowType>(kmsScreen->surface());
    EGLSurface surface = eglCreateWindowSurface(display, m_config, window, NULL);
    if (Q_UNLIKELY(surface == EGL_NO_SURFACE)) {
        EGLint error = eglGetError();
        eglTerminate(display);
        qFatal("EGL Error : Could not create a new egl surface: error = 0x%x\n", error);
        return;
    }

    if (m_surface != EGL_NO_SURFACE) {
        EGLDisplay display = static_cast<EglFSScreen *>(screen())->display();
        eglDestroySurface(display, m_surface);
    }

    m_window = window;
    m_surface = surface;

    kmsScreen->setPrimarySurface(m_surface);
}

} // namespace Platform

} // namespace GreenIsland
