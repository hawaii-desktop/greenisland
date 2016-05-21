/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
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
    void runGLchecks() Q_DECL_OVERRIDE;
    void swapBuffers(QPlatformSurface *surface) Q_DECL_OVERRIDE;

private:
    EGLNativeWindowType m_tempWindow;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSCONTEXT_H
