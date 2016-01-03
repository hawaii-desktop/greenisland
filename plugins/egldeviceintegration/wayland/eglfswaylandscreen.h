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

#ifndef GREENISLAND_EGLFSWAYLANDSCREEN_H
#define GREENISLAND_EGLFSWAYLANDSCREEN_H

#include <GreenIsland/Client/Output>

#include <GreenIsland/Platform/EglFSScreen>

#include "eglfswaylandintegration.h"

namespace GreenIsland {

namespace Platform {

class EglFSWaylandScreen : public EglFSScreen
{
public:
    EglFSWaylandScreen(EGLDisplay display,
                       EglFSWaylandIntegration *integration,
                       Client::Output *output);

    QRect geometry() const Q_DECL_OVERRIDE;
    int depth() const Q_DECL_OVERRIDE;
    QImage::Format format() const Q_DECL_OVERRIDE;

    QSizeF physicalSize() const Q_DECL_OVERRIDE;
    QDpi logicalDpi() const Q_DECL_OVERRIDE;
    Qt::ScreenOrientation nativeOrientation() const Q_DECL_OVERRIDE;
    Qt::ScreenOrientation orientation() const Q_DECL_OVERRIDE;

    QString name() const Q_DECL_OVERRIDE;

    QPlatformCursor *cursor() const Q_DECL_OVERRIDE;

    qreal refreshRate() const Q_DECL_OVERRIDE;

    QList<QPlatformScreen *> virtualSiblings() const Q_DECL_OVERRIDE { return m_siblings; }
    void setVirtualSiblings(QList<QPlatformScreen *> sl) { m_siblings = sl; }

    EglFSWaylandIntegration *integration() const { return m_integration; }
    Client::Output *output() const { return m_output; }

private:
    EglFSWaylandIntegration *m_integration;
    Client::Output *m_output;
    QList<QPlatformScreen *> m_siblings;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSWAYLANDSCREEN_H
