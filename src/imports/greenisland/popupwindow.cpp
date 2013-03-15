/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QTimer>

#include "popupwindow.h"

PopupWindow::PopupWindow(QWindow *parent)
    : QQuickWindow(parent)
{
    // This is also a custom window (no shell surface)
    setFlags(Qt::Popup);

    // Set desired surface format
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setAlphaBufferSize(8);
    setFormat(format);

    // Clear before rendering
    setColor(Qt::transparent);
    setClearBeforeRendering(true);
}

#include "moc_popupwindow.cpp"
