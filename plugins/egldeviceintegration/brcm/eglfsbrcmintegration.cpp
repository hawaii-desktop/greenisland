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

#include "eglfsbrcmintegration.h"

#include <bcm_host.h>

namespace GreenIsland {

namespace Platform {

static DISPMANX_DISPLAY_HANDLE_T dispman_display = 0;

static EGLNativeWindowType createDispmanxLayer(const QPoint &pos, const QSize &size, int z, DISPMANX_FLAGS_ALPHA_T flags)
{
    VC_RECT_T dst_rect;
    dst_rect.x = pos.x();
    dst_rect.y = pos.y();
    dst_rect.width = size.width();
    dst_rect.height = size.height();

    VC_RECT_T src_rect;
    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = size.width() << 16;
    src_rect.height = size.height() << 16;

    DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);

    VC_DISPMANX_ALPHA_T alpha;
    alpha.flags = flags;
    alpha.opacity = 0xFF;
    alpha.mask = 0;

    DISPMANX_ELEMENT_HANDLE_T dispman_element = vc_dispmanx_element_add(
            dispman_update, dispman_display, z, &dst_rect, 0, &src_rect,
            DISPMANX_PROTECTION_NONE, &alpha, (DISPMANX_CLAMP_T *)NULL, (DISPMANX_TRANSFORM_T)0);

    vc_dispmanx_update_submit_sync(dispman_update);

    EGL_DISPMANX_WINDOW_T *eglWindow = new EGL_DISPMANX_WINDOW_T;
    eglWindow->element = dispman_element;
    eglWindow->width = size.width();
    eglWindow->height = size.height();

    return eglWindow;
}

static void destroyDispmanxLayer(EGLNativeWindowType window)
{
    EGL_DISPMANX_WINDOW_T *eglWindow = static_cast<EGL_DISPMANX_WINDOW_T *>(window);
    DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
    vc_dispmanx_element_remove(dispman_update, eglWindow->element);
    vc_dispmanx_update_submit_sync(dispman_update);
    delete eglWindow;
}

void EglFSBrcmIntegration::platformInit()
{
    bcm_host_init();
}

EGLNativeDisplayType EglFSBrcmIntegration::platformDisplay() const
{
    dispman_display = vc_dispmanx_display_open(0/* LCD */);
    return EGL_DEFAULT_DISPLAY;
}

void EglFSBrcmIntegration::platformDestroy()
{
    vc_dispmanx_display_close(dispman_display);
}

QSize EglFSBrcmIntegration::screenSize() const
{
    uint32_t width, height;
    graphics_get_display_size(0 /* LCD */, &width, &height);
    return QSize(width, height);
}

EGLNativeWindowType EglFSBrcmIntegration::createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format)
{
    Q_UNUSED(window)
    return createDispmanxLayer(QPoint(0, 0), size, 1, format.hasAlpha() ? DISPMANX_FLAGS_ALPHA_FROM_SOURCE : DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS);
}

void EglFSBrcmIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    destroyDispmanxLayer(window);
}

bool EglFSBrcmIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
        case QPlatformIntegration::ThreadedPixmaps:
        case QPlatformIntegration::OpenGL:
        case QPlatformIntegration::ThreadedOpenGL:
        case QPlatformIntegration::BufferQueueingOpenGL:
            return true;
        default:
            return false;
    }
}

} // namespace Platform

} // namespace GreenIsland
