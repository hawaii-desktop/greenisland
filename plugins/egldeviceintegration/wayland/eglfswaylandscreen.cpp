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
