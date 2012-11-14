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

#include <QGuiApplication>
#include <QScreen>
#include <QQmlContext>
#include <QQuickItem>
#include <QtCompositor/waylandsurface.h>
#include <QtCompositor/waylandsurfaceitem.h>
#include <QtCompositor/waylandinput.h>

#include "desktopshell.h"
#include "cmakedirs.h"

DesktopShell::DesktopShell()
    : VShell()
{
    // Load the shell
    setSource(QUrl("qrc:///qml/Shell.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::transparent);

    // All the screen is available
    m_availableGeometry = geometry();

    // Allow QML to access this shell
    rootContext()->setContextProperty("shell", this);

    setGeometry(0, 0, 1024, 768);
    show();
}

QRectF DesktopShell::availableGeometry() const
{
    return m_availableGeometry;
}

void DesktopShell::setAvailableGeometry(const QRectF &rect)
{
    m_availableGeometry = rect;
    emit availableGeometryChanged();
}

#if 0
void DesktopShell::startShell()
{
    // Set path so that programs will be found
    QByteArray path = qgetenv("PATH");
    if (!path.isEmpty())
        path += ":";
    path += INSTALL_BINDIR;
    setenv("PATH", qPrintable(path), 1);

    // Force QPA to wayland
    setenv("QT_QPA_PLATFORM", "wayland", 1);

    // Force GTK+ backend to wayland
    setenv("GDK_BACKEND", "wayland", 1);
}
#endif

#include "moc_desktopshell.cpp"
