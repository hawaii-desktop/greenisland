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

#ifndef GREENISLAND_EGLPBUFFER_H
#define GREENISLAND_EGLPBUFFER_H

#include <EGL/egl.h>
#include <QtGui/qpa/qplatformoffscreensurface.h>

#include <GreenIsland/Platform/EGLPlatformContext>

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT EGLPbuffer : public QPlatformOffscreenSurface
{
public:
    EGLPbuffer(EGLDisplay display, const QSurfaceFormat &format, QOffscreenSurface *offscreenSurface,
                EGLPlatformContext::Flags flags = 0);
    ~EGLPbuffer();

    QSurfaceFormat format() const Q_DECL_OVERRIDE { return m_format; }
    bool isValid() const Q_DECL_OVERRIDE { return m_pbuffer != EGL_NO_SURFACE; }

    EGLSurface pbuffer() const { return m_pbuffer; }

private:
    QSurfaceFormat m_format;
    EGLDisplay m_display;
    EGLSurface m_pbuffer;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLPBUFFER_H
