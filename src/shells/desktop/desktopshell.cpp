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

#include <QGuiApplication>
#include <QScreen>

#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

#include "cmakedirs.h"
#include "desktopshell.h"
#include "desktopshellintegration.h"
#include "shellview.h"

const struct wl_registry_listener DesktopShell::registryListener = {
    DesktopShellIntegration::handleGlobal
};

Q_GLOBAL_STATIC(DesktopShell, desktopShell)

DesktopShell::DesktopShell()
    : QObject()
{
    // Set path so that programs will be found
    QByteArray path = qgetenv("PATH");
    if (!path.isEmpty())
        path += ":";
    path += INSTALL_BINDIR;
    setenv("PATH", qPrintable(path), 1);

    // Get the Wayland display and registry
    QPlatformNativeInterface *native =
        QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);
    struct wl_display *display = (struct wl_display *)
                                 native->nativeResourceForIntegration("display");
    Q_ASSERT(display);
    struct wl_registry *registry = wl_display_get_registry(display);

    // Wayland integration
    DesktopShellIntegration::createInstance(this);
    wl_registry_add_listener(registry, &DesktopShell::registryListener,
                             DesktopShellIntegration::instance());

    // Create the shell window
    m_shellView = new ShellView(this);
    m_shellView->setGeometry(QGuiApplication::primaryScreen()->availableGeometry());
    m_shellView->showFullScreen();
}

DesktopShell::~DesktopShell()
{
    delete m_shellView;
}

DesktopShell *DesktopShell::instance()
{
    return desktopShell();
}

void DesktopShell::updateAvailableGeometry()
{
    DesktopShellIntegration::instance()->updateAvailableGeometry();
}

#include "moc_desktopshell.cpp"
