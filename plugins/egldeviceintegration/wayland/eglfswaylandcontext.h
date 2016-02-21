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

#ifndef GREENISLAND_EGLFSWAYLANDCONTEXT_H
#define GREENISLAND_EGLFSWAYLANDCONTEXT_H

#include <QtCore/QVariant>

#include <GreenIsland/Platform/EGLPlatformContext>

#include "eglfswaylandintegration.h"

namespace GreenIsland {

namespace Platform {

class EglFSWaylandBlitter;

class EglFSWaylandContext : public EGLPlatformContext
{
public:
    EglFSWaylandContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share,
                        EGLDisplay display, EGLConfig *config,
                        const QVariant &nativeHandle);
    virtual ~EglFSWaylandContext();

    bool makeCurrent(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    void swapBuffers(QPlatformSurface *surface) Q_DECL_OVERRIDE;

    GLuint defaultFramebufferObject(QPlatformSurface *surface) const Q_DECL_OVERRIDE;

protected:
    EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    EGLSurface createTemporaryOffscreenSurface() Q_DECL_OVERRIDE;
    void destroyTemporaryOffscreenSurface(EGLSurface surface) Q_DECL_OVERRIDE;

private:
    EglFSWaylandBlitter *m_blitter;
    bool m_useNativeDefaultFbo;

    friend class EglFSWaylandBlitter;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSWAYLANDCONTEXT_H
