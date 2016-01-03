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

#include "eglfsmaliintegration.h"

#include <EGL/fbdev_window.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include <private/qcore_unix_p.h>

namespace GreenIsland {

namespace Platform {

void EglFSMaliIntegration::platformInit()
{
    // Keep the non-overridden base class functions based on fb0 working.
    QEGLDeviceIntegration::platformInit();

    int fd = qt_safe_open("/dev/fb0", O_RDWR, 0);
    if (fd == -1)
        qWarning("Failed to open fb to detect screen resolution!");

    struct fb_var_screeninfo vinfo;
    memset(&vinfo, 0, sizeof(vinfo));
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
        qWarning("Could not get variable screen info");

    vinfo.bits_per_pixel   = 32;
    vinfo.red.length       = 8;
    vinfo.green.length     = 8;
    vinfo.blue.length      = 8;
    vinfo.transp.length    = 8;
    vinfo.blue.offset      = 0;
    vinfo.green.offset     = 8;
    vinfo.red.offset       = 16;
    vinfo.transp.offset    = 24;
#if 0
    vinfo.yres_virtual     = 2 * vinfo.yres;
#endif

    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) == -1)
        qErrnoWarning(errno, "Unable to set double buffer mode!");

    qt_safe_close(fd);
}

EGLNativeWindowType EglFSMaliIntegration::createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format)
{
    Q_UNUSED(window);
    Q_UNUSED(format);

    fbdev_window *fbwin = reinterpret_cast<fbdev_window *>(malloc(sizeof(fbdev_window)));
    if (NULL == fbwin)
        return 0;

    fbwin->width = size.width();
    fbwin->height = size.height();
    return (EGLNativeWindowType)fbwin;
}

void EglFSMaliIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    free((void*)window);
}

} // namespace Platform

} // namespace GreenIsland
