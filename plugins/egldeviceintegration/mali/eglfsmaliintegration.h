/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef GREENISLAND_EGLFSMALIINTEGRATION_H
#define GREENISLAND_EGLFSMALIINTEGRATION_H

#include <GreenIsland/Platform/EGLDeviceIntegration>

namespace GreenIsland {

namespace Platform {

class EglFSMaliIntegration : public EGLDeviceIntegration
{
public:
    void platformInit() Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    void destroyNativeWindow(EGLNativeWindowType window) Q_DECL_OVERRIDE;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSMALIINTEGRATION_H
