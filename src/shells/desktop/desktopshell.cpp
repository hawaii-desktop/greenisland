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

#include <QDebug>
#include <QGuiApplication>
#include <QSocketNotifier>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

#include "cmakedirs.h"
#include "desktopshell.h"
#include "waylandintegration.h"
#include "launcher.h"
#include "background.h"

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

    // Platform native interface
    QPlatformNativeInterface *native =
        QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);

    // Get the Wayland display and registry
    m_display = static_cast<struct wl_display *>(
                    native->nativeResourceForIntegration("display"));
    Q_ASSERT(m_display);
    m_fd = wl_display_get_fd(m_display);
    m_registry = wl_display_get_registry(m_display);
    Q_ASSERT(m_registry);

    // Wayland integration
    WaylandIntegration *integration = WaylandIntegration::createInstance(this);
    wl_list_init(&integration->outputs);
    wl_registry_add_listener(m_registry, &WaylandIntegration::registryListener,
                             WaylandIntegration::instance());

    QSocketNotifier *readNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(readNotifier, SIGNAL(activated(int)), this, SLOT(readEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(awake()), this, SLOT(flushRequests()));

    QElapsedTimer timeout;
    timeout.start();
    do {
        QGuiApplication::processEvents();
    } while (m_screens.isEmpty() && timeout.elapsed() < 1000);

    if (m_screens.isEmpty())
        qFatal("Failed to receive globals from display");

    Output *output;
    wl_list_for_each(output, &integration->outputs, link) {
        // Set a wallpaper for each output
        output->background = new Background();
        output->backgroundSurface = static_cast<struct wl_surface *>(
                    native->nativeResourceForWindow("surface", output->background->window()));
        wl_surface_set_user_data(output->backgroundSurface, output->background);
        desktop_shell_set_background(integration->shell, output->output,
                                     output->backgroundSurface);
        continue;

        // Create a launcher window for each output
        output->launcher = new Launcher();
        output->launcherSurface = static_cast<struct wl_surface *>(
                    native->nativeResourceForWindow("surface", output->launcher));
        wl_surface_set_user_data(output->launcherSurface, output->launcher);
        output->launcher->setGeometry(QRect(0, 0, 1, 1));
        desktop_shell_set_panel(integration->shell, output->output,
                                output->launcherSurface);
    }
}

DesktopShell *DesktopShell::instance()
{
    return desktopShell();
}

void DesktopShell::addScreen(Output *output)
{
    m_screens.append(output);
}

void DesktopShell::readEvents()
{
    wl_display_dispatch(m_display);
}

void DesktopShell::flushRequests()
{
    wl_display_dispatch_pending(m_display);
    wl_display_flush(m_display);
}

#include "moc_desktopshell.cpp"
