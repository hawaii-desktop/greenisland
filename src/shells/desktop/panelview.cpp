/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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

#include <QWindow>
#include <QQmlContext>

#include "panelview.h"

PanelView::PanelView(VShell *shell)
    : ShellQuickView(shell)
{
    // This is a frameless window that stays on top of everything
    parent()->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    // Make it transparent
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setSamples(16);
    surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));

    // Set context properties
    rootContext()->setContextProperty("shell", shell);
    rootContext()->setContextProperty("quickview", this);

    // Load QML view
    setSource(QUrl("qrc:///qml/Panel.qml"));
    setResizeMode(QQuickView::SizeViewToRootObject);
}

#include "moc_panelview.cpp"
