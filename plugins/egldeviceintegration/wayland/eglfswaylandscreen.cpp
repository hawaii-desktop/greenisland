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

#include "eglfswaylandintegration.h"
#include "eglfswaylandscreen.h"

namespace GreenIsland {

namespace Platform {

EglFSWaylandScreen::EglFSWaylandScreen(EGLDisplay display,
                                       EglFSWaylandIntegration *integration,
                                       Client::Output *output)
    : EglFSScreen(display)
    , m_integration(integration)
    , m_output(output)
{
}

QRect EglFSWaylandScreen::geometry() const
{
    return m_output->geometry();
}

int EglFSWaylandScreen::depth() const
{
    return 32;
}

QImage::Format EglFSWaylandScreen::format() const
{
    return QImage::Format_RGB32;
}

QSizeF EglFSWaylandScreen::physicalSize() const
{
    return m_output->physicalSize();
}

QDpi EglFSWaylandScreen::logicalDpi() const
{
    QSizeF ps = physicalSize();
    QSize s = geometry().size();

    if (ps.isValid() && s.isValid())
        return QDpi(25.4 * s.width() / ps.width(),
                    25.4 * s.height() / ps.height());
    else
        return QDpi(100, 100);
}

Qt::ScreenOrientation EglFSWaylandScreen::nativeOrientation() const
{
    return Qt::PrimaryOrientation;
}

Qt::ScreenOrientation EglFSWaylandScreen::orientation() const
{
    return Qt::PrimaryOrientation;
}

QString EglFSWaylandScreen::name() const
{
    return m_output->model();
}

QPlatformCursor *EglFSWaylandScreen::cursor() const
{
    return Q_NULLPTR;
}

qreal EglFSWaylandScreen::refreshRate() const
{
    return m_output->refreshRate();
}

} // namespace Platform

} // namespace GreenIsland
