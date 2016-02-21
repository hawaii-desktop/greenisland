/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
