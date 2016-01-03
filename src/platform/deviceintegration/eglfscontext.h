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

#ifndef GREENISLAND_EGLFSCONTEXT_H
#define GREENISLAND_EGLFSCONTEXT_H

#include <QtCore/QVariant>

#include <GreenIsland/Platform/EGLPlatformContext>

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT EglFSContext : public EGLPlatformContext
{
public:
    EglFSContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display,
                 EGLConfig *config, const QVariant &nativeHandle);

    EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    EGLSurface createTemporaryOffscreenSurface() Q_DECL_OVERRIDE;
    void destroyTemporaryOffscreenSurface(EGLSurface surface) Q_DECL_OVERRIDE;
    void swapBuffers(QPlatformSurface *surface) Q_DECL_OVERRIDE;

private:
    EGLNativeWindowType m_tempWindow;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSCONTEXT_H
