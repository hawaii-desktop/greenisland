/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
 ***************************************************************************/

#include <GreenIsland/Platform/EGLPbuffer>

#include "eglfswaylandcontext.h"
#include "eglfswaylandblitter.h"
#include "eglfswaylandwindow.h"
#include "eglfswaylandlogging.h"
#include "stateguard.h"

namespace GreenIsland {

namespace Platform {

EglFSWaylandContext::EglFSWaylandContext(const QSurfaceFormat &format,
                                         QPlatformOpenGLContext *share,
                                         EGLDisplay display,
                                         EGLConfig *config,
                                         const QVariant &nativeHandle)
    : EGLPlatformContext(format, share, display, config, nativeHandle)
    , m_blitter(Q_NULLPTR)
    , m_useNativeDefaultFbo(false)
{
}

EglFSWaylandContext::~EglFSWaylandContext()
{
    delete m_blitter;
    eglDestroyContext(eglDisplay(), eglContext());
}

bool EglFSWaylandContext::makeCurrent(QPlatformSurface *surface)
{
    bool result = EGLPlatformContext::makeCurrent(surface);

    EglFSWaylandWindow *window = static_cast<EglFSWaylandWindow *>(surface);
    window->bindContentFBO();

    return result;
}

void EglFSWaylandContext::swapBuffers(QPlatformSurface *surface)
{
    EglFSWaylandWindow *window = static_cast<EglFSWaylandWindow *>(surface);
    EGLSurface eglSurface = window->surface();

    makeCurrent(surface);

    StateGuard stateGuard;

    if (!m_blitter)
        m_blitter = new EglFSWaylandBlitter(this);
    m_blitter->blit(window);

    eglSwapInterval(eglDisplay(), format().swapInterval());
    eglSwapBuffers(eglDisplay(), eglSurface);

    //window.setCanResize(true);
}

GLuint EglFSWaylandContext::defaultFramebufferObject(QPlatformSurface *surface) const
{
    if (m_useNativeDefaultFbo)
        return 0;

    return static_cast<EglFSWaylandWindow *>(surface)->contentFBO();
}

EGLSurface EglFSWaylandContext::eglSurfaceForPlatformSurface(QPlatformSurface *surface)
{
    if (surface->surface()->surfaceClass() == QSurface::Window)
        return static_cast<EglFSWaylandWindow *>(surface)->surface();
    else
        return static_cast<EGLPbuffer *>(surface)->pbuffer();
}

EGLSurface EglFSWaylandContext::createTemporaryOffscreenSurface()
{
    return EGLPlatformContext::createTemporaryOffscreenSurface();
}

void EglFSWaylandContext::destroyTemporaryOffscreenSurface(EGLSurface surface)
{
    EGLPlatformContext::destroyTemporaryOffscreenSurface(surface);
}

} // namespace Platform

} // namespace GreenIsland
