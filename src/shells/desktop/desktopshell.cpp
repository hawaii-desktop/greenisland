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

#include "desktopshell.h"
#include "cmakedirs.h"

DesktopShell::DesktopShell()
    : VShell(new QWindow())
{
    // Make it transparent
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setSamples(16);
    surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setResizeMode(QQuickView::SizeViewToRootObject);

    // Set path so that programs will be found
    QByteArray path = qgetenv("PATH");
    if (!path.isEmpty())
        path += ":";
    path += INSTALL_BINDIR;
    setenv("PATH", qPrintable(path), 1);

    // All the screen is available
    m_availableGeometry = screenGeometry();

    // Intercept screen geometry changes
    connect(screen(), SIGNAL(virtualGeometryChanged(QRect)),
            this, SIGNAL(screenGeometryChanged()));

    // Allow QML to access this shell
    rootContext()->setContextProperty("shell", this);

    // Load the shell
    setSource(QUrl("qrc:///qml/Shell.qml"));
}

QRectF DesktopShell::screenGeometry() const
{
    return screen()->availableGeometry();
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

#include "moc_desktopshell.cpp"
