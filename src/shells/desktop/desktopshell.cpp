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

#include "cmakedirs.h"
#include "desktopshell.h"
#include "panelview.h"
#include "launcherview.h"

DesktopShell::DesktopShell()
    : VShell()
{
#if 0
    // Make it transparent
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setSamples(16);
    surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setResizeMode(QQuickView::SizeViewToRootObject);
#endif

    // Set path so that programs will be found
    QByteArray path = qgetenv("PATH");
    if (!path.isEmpty())
        path += ":";
    path += INSTALL_BINDIR;
    setenv("PATH", qPrintable(path), 1);

    // All the screen is available
    m_availableGeometry = screenGeometry();

    // Panel
    m_panelView = new PanelView(this);

    // Launcher
    m_launcherView = new LauncherView(this);
    m_launcherView->configure();
}

DesktopShell::~DesktopShell()
{
    delete m_launcherView;
    delete m_panelView;
}

QRectF DesktopShell::screenGeometry() const
{
#if 0
    return m_screen->availableGeometry();
#endif
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

void DesktopShell::show()
{
    m_panelView->show();
    m_launcherView->show();
}

void DesktopShell::hide()
{
    m_launcherView->hide();
    m_panelView->hide();
}

#include "moc_desktopshell.cpp"
