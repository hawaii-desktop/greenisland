/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2015 The Qt Company Ltd.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in
 *         the documentation and/or other materials provided with the
 *         distribution.
 *       * Neither the name of The Qt Company Ltd nor the names of its
 *         contributors may be used to endorse or promote products derived
 *         from this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include <QtCore/QDebug>
#include <QtCore/QPointer>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qplatformscreen.h>

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwlsurface_p.h"

#include "waylandeglclientbufferintegration.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

/* Needed for compatibility with Mesa older than 10.0. */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYWAYLANDBUFFERWL_compat) (EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);

#ifndef EGL_WL_bind_wayland_display
typedef EGLBoolean (EGLAPIENTRYP PFNEGLBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
#endif

#ifndef EGL_KHR_image
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLeglImageOES image);
#endif

class WaylandEglClientBufferIntegrationPrivate
{
public:
    WaylandEglClientBufferIntegrationPrivate()
        : egl_display(EGL_NO_DISPLAY)
        , valid(false)
        , display_bound(false)
        , egl_bind_wayland_display(0)
        , egl_unbind_wayland_display(0)
        , egl_query_wayland_buffer(0)
        , egl_create_image(0)
        , egl_destroy_image(0)
        , gl_egl_image_target_texture_2d(0)
    { }
    EGLDisplay egl_display;
    bool valid;
    bool display_bound;
    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display;
    PFNEGLQUERYWAYLANDBUFFERWL_compat egl_query_wayland_buffer;

    PFNEGLCREATEIMAGEKHRPROC egl_create_image;
    PFNEGLDESTROYIMAGEKHRPROC egl_destroy_image;

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gl_egl_image_target_texture_2d;
};

WaylandEglClientBufferIntegration::WaylandEglClientBufferIntegration()
    : GreenIsland::ClientBufferIntegration()
    , d_ptr(new WaylandEglClientBufferIntegrationPrivate)
{
}

void WaylandEglClientBufferIntegration::initializeHardware(GreenIsland::Display *waylandDisplay)
{
    Q_D(WaylandEglClientBufferIntegration);

    const bool ignoreBindDisplay = !qgetenv("QT_WAYLAND_IGNORE_BIND_DISPLAY").isEmpty();

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qWarning("GreenIsland: Failed to initialize EGL display. No native platform interface available.");
        return;
    }

    d->egl_display = nativeInterface->nativeResourceForIntegration("EglDisplay");
    if (!d->egl_display) {
        qWarning("GreenIsland: Failed to initialize EGL display. Could not get EglDisplay for window.");
        return;
    }

    const char *extensionString = eglQueryString(d->egl_display, EGL_EXTENSIONS);
    if ((!extensionString || !strstr(extensionString, "EGL_WL_bind_wayland_display")) && !ignoreBindDisplay) {
        qWarning("GreenIsland: Failed to initialize EGL display. There is no EGL_WL_bind_wayland_display extension.");
        return;
    }

    d->egl_bind_wayland_display = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
    d->egl_unbind_wayland_display = reinterpret_cast<PFNEGLUNBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
    if ((!d->egl_bind_wayland_display || !d->egl_unbind_wayland_display) && !ignoreBindDisplay) {
        qWarning("GreenIsland: Failed to initialize EGL display. Could not find eglBindWaylandDisplayWL and eglUnbindWaylandDisplayWL.");
        return;
    }

    d->egl_query_wayland_buffer = reinterpret_cast<PFNEGLQUERYWAYLANDBUFFERWL_compat>(eglGetProcAddress("eglQueryWaylandBufferWL"));
    if (!d->egl_query_wayland_buffer) {
        qWarning("GreenIsland: Failed to initialize EGL display. Could not find eglQueryWaylandBufferWL.");
        return;
    }

    d->egl_create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    d->egl_destroy_image = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!d->egl_create_image || !d->egl_destroy_image) {
        qWarning("GreenIsland: Failed to initialize EGL display. Could not find eglCreateImageKHR and eglDestroyImageKHR.");
        return;
    }

    d->gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!d->gl_egl_image_target_texture_2d) {
        qWarning("GreenIsland: Failed to initialize EGL display. Could not find glEGLImageTargetTexture2DOES.");
        return;
    }

    if (d->egl_bind_wayland_display && d->egl_unbind_wayland_display) {
        d->display_bound = d->egl_bind_wayland_display(d->egl_display, waylandDisplay->handle());
        if (!d->display_bound) {
            if (!ignoreBindDisplay) {
                qWarning("GreenIsland: Failed to initialize EGL display. Could not bind Wayland display.");
                return;
            } else {
                qWarning("GreenIsland: Could not bind Wayland display. Ignoring.");
            }
        }
    }

    d->valid = true;
}

void WaylandEglClientBufferIntegration::bindTextureToBuffer(struct ::wl_resource *buffer)
{
    Q_D(WaylandEglClientBufferIntegration);
    if (!d->valid) {
        qWarning("GreenIsland: bindTextureToBuffer() failed");
        return;
    }

    EGLImageKHR image = d->egl_create_image(d->egl_display, EGL_NO_CONTEXT,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);

    d->gl_egl_image_target_texture_2d(GL_TEXTURE_2D, image);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    d->egl_destroy_image(d->egl_display, image);
}

bool WaylandEglClientBufferIntegration::isYInverted(struct ::wl_resource *buffer) const
{
#if defined(EGL_WAYLAND_Y_INVERTED_WL)
    Q_D(const WaylandEglClientBufferIntegration);

    EGLint isYInverted;
    EGLBoolean ret;
    ret = d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WAYLAND_Y_INVERTED_WL, &isYInverted);

    // Yes, this looks strange, but the specification says that EGL_FALSE return
    // value (not supported) should be treated the same as EGL_TRUE return value
    // and EGL_TRUE in value.
    if (ret == EGL_FALSE || isYInverted == EGL_TRUE)
        return true;
    return false;
#endif

    return GreenIsland::ClientBufferIntegration::isYInverted(buffer);
}


void *WaylandEglClientBufferIntegration::lockNativeBuffer(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    EGLImageKHR image = d->egl_create_image(d->egl_display, EGL_NO_CONTEXT,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);
    return image;
}

void WaylandEglClientBufferIntegration::unlockNativeBuffer(void *native_buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);
    EGLImageKHR image = static_cast<EGLImageKHR>(native_buffer);

    d->egl_destroy_image(d->egl_display, image);
}

QSize WaylandEglClientBufferIntegration::bufferSize(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    int width, height;
    d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WIDTH, &width);
    d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_HEIGHT, &height);

    return QSize(width, height);
}
