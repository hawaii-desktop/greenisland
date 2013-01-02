/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#include <QScreen>

#include "shellquickview.h"

ShellQuickView::ShellQuickView(DesktopShell *shell)
    : QQuickView(new QWindow())
    , m_shell(shell)
{
    // Intercept screen geometry changes
    connect(screen(), SIGNAL(geometryChanged(QRect)),
            this, SIGNAL(screenGeometryChanged()));
    connect(screen(), SIGNAL(virtualGeometryChanged(QRect)),
            this, SIGNAL(screenGeometryChanged()));
}

QRectF ShellQuickView::screenGeometry() const
{
    return screen()->availableGeometry();
}

#include "moc_shellquickview.cpp"
