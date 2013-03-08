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
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>

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

    // QML engine
    m_qmlEngine = new QQmlEngine(this);

    // Platform native interface
    QPlatformNativeInterface *native =
            QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);

    // Get Wayland display
    m_display = static_cast<struct wl_display *>(
                native->nativeResourceForIntegration("display"));
    Q_ASSERT(m_display);

    // Display file descriptor
    m_fd = wl_display_get_fd(m_display);
    Q_ASSERT(m_fd > -1);
    qDebug() << "Wayland display socket:" << m_fd;

    // Wayland registry
    m_registry = wl_display_get_registry(m_display);
    Q_ASSERT(m_registry);

    // Wayland integration
    WaylandIntegration *integration = WaylandIntegration::instance();
    wl_registry_add_listener(m_registry, &WaylandIntegration::registryListener,
                             integration);
}

DesktopShell::~DesktopShell()
{
    foreach (Output *output, m_outputs) {
        m_outputs.removeOne(output);
        delete output;
    }

    delete m_qmlEngine;
}

DesktopShell *DesktopShell::instance()
{
    return desktopShell();
}

void DesktopShell::create()
{
    // Platform native interface
    QPlatformNativeInterface *native =
            QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);

    // Wayland integration
    WaylandIntegration *object = WaylandIntegration::instance();

    foreach (QScreen *screen, QGuiApplication::screens()) {
        Output *output = new Output();

        // Get native wl_output for the current screen
        output->screen = screen;
        output->output = static_cast<struct wl_output *>(
                    native->nativeResourceForScreen("output", output->screen));

        // Geometry
        qDebug() << "Creating shell surfaces on" << screen->name()
                 << "with geometry" << screen->geometry();

        // Set a wallpaper for each screen
        output->background = new Background(screen, this);
        output->backgroundSurface = static_cast<struct wl_surface *>(
                    native->nativeResourceForWindow("surface",
                                                    output->background->window()));
        desktop_shell_set_background(object->shell, output->output,
                                     output->backgroundSurface);
        qDebug() << "Created background surface" << output->backgroundSurface
                 << "for output" << output->output;

        // Create a launcher window for each output
        output->launcher = new Launcher(screen, this);
        output->launcherSurface = static_cast<struct wl_surface *>(
                    native->nativeResourceForWindow("surface",
                                                    output->launcher->window()));
        desktop_shell_set_panel(object->shell, output->output,
                                output->launcherSurface);
        qDebug() << "Created launcher surface" << output->launcherSurface
                 << "for output" << output->output;

        addOutput(output);
    }
}

#include "moc_desktopshell.cpp"
