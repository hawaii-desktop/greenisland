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

#include <QSurfaceFormat>
#include <QQmlContext>
#include <QQuickItem>

#include "launcher.h"

Launcher::Launcher()
    : QQuickView(QUrl("qrc:///qml/Launcher.qml"))
{
    // This is a frameless window that stays on top of everything
    setTitle(QLatin1String("Hawaii Launcher"));
    setFlags(Qt::FramelessWindowHint | Qt::CustomWindow);

    // Resize root item to this view
    setResizeMode(QQuickView::SizeRootObjectToView);

    // Set context properties
    rootContext()->setContextProperty("quickview", this);

    // Make it transparent
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setSamples(16);
    surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    winId();
}

int Launcher::tileSize() const
{
    return rootObject()->property("tileSize").toInt();
}

#include "moc_launcher.cpp"
