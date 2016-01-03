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

#include "eglfsvivintegration.h"

#include <EGL/eglvivante.h>
#include <QDebug>

#include <wayland-server.h>

namespace GreenIsland {

namespace Platform {

void EglFSVivIntegration::platformInit()
{
    QEGLDeviceIntegration::platformInit();

    int width, height;

    bool multiBufferNotEnabledYet = qEnvironmentVariableIsEmpty("FB_MULTI_BUFFER");
    bool multiBuffer = qEnvironmentVariableIsEmpty("QT_EGLFS_IMX6_NO_FB_MULTI_BUFFER");
    if (multiBufferNotEnabledYet && multiBuffer) {
        qWarning() << "EglFSVivIntegration will set environment variable FB_MULTI_BUFFER=2 to enable double buffering and vsync.\n"
                   << "If this is not desired, you can override this via: export QT_EGLFS_IMX6_NO_FB_MULTI_BUFFER=1";
        qputenv("FB_MULTI_BUFFER", "2");
    }

    mWaylandDisplay = wl_display_create();
    mNativeDisplay = fbGetDisplayByIndex(framebufferIndex());
    fbGetDisplayGeometry(mNativeDisplay, &width, &height);
    mScreenSize.setHeight(height);
    mScreenSize.setWidth(width);
}

QSize EglFSVivIntegration::screenSize() const
{
    return mScreenSize;
}

void *EglFSVivIntegration::wlDisplay() const
{
    return mWaylandDisplay;
}

EGLNativeDisplayType EglFSVivIntegration::platformDisplay() const
{
    return mNativeDisplay;
}

EGLNativeWindowType EglFSVivIntegration::createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format)
{
    Q_UNUSED(window)
    Q_UNUSED(format)

    EGLNativeWindowType eglWindow = fbCreateWindow(mNativeDisplay, 0, 0, size.width(), size.height());
    return eglWindow;
}

void EglFSVivIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    fbDestroyWindow(window);
}

} // namespace Platform

} // namespace GreenIsland
