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

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QQmlContext>
#include <QQuickItem>
#include <qpa/qplatformnativeinterface.h>

#include "launcherview.h"
#include "desktopshellintegration.h"

LauncherView::LauncherView(VShell *shell)
    : ShellQuickView(shell)
{
    // This is a frameless window that stays on top of everything
    setTitle(QLatin1String("Launcher"));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

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
    setSource(QUrl("qrc:///qml/Launcher.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);

    // Load settings
    m_settings = new VSettings("org.hawaii.greenisland");
    connect(m_settings, SIGNAL(changed()), this, SLOT(settingsChanged()));
    settingsChanged();
}

void LauncherView::configure()
{
    QPlatformNativeInterface *native =
        QGuiApplication::platformNativeInterface();
    if (!native)
        return;

#if 0
    // Make this window special for the compositor
    native->setWindowProperty(handle(), "type", "launcher");

    // Pass the coordinates to the compositor
    native->setWindowProperty(handle(), "position", position());
#endif
}

void LauncherView::settingsChanged()
{
    QString alignment = m_settings->value("launcher/alignment").toString();
    qreal launcherSize = rootObject()->property("launcherSize").toReal();
    QRect rect = screen()->availableGeometry();

    if (alignment == "left")
        setGeometry(rect.x(), rect.y(),
                    launcherSize, rect.height() - rect.y());
    else if (alignment == "right")
        setGeometry(rect.x(), rect.y(),
                    launcherSize, rect.height() - rect.y());
    else if (alignment == "bottom")
        setGeometry(rect.x(), rect.height() - launcherSize,
                    rect.width(), launcherSize);

    QPlatformNativeInterface *native =
        QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);
    struct wl_surface *surface = (struct wl_surface *)
                                 native->nativeResourceForWindow("surface", this);
    Q_ASSERT(surface);
    Q_ASSERT(DesktopShellIntegration::instance()->shell);
    desktop_shell_set_launcher(DesktopShellIntegration::instance()->shell,
                               surface);
    desktop_shell_set_launcher_pos(DesktopShellIntegration::instance()->shell,
                                   geometry().x(), geometry().y());
}

#include "moc_launcherview.cpp"
