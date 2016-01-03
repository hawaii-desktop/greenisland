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
