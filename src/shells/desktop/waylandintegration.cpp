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
#include <QScreen>

#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>

#include "waylandintegration.h"
#include "desktopshell.h"
#include "launcher.h"
#include "background.h"

Q_GLOBAL_STATIC(WaylandIntegration, s_waylandIntegration)

const struct wl_registry_listener WaylandIntegration::registryListener = {
    WaylandIntegration::handleGlobal
};

const struct desktop_shell_listener WaylandIntegration::listener = {
    WaylandIntegration::handleConfigure,
    WaylandIntegration::handlePrepareLockSurface,
    WaylandIntegration::handleGrabCursor
};

WaylandIntegration::WaylandIntegration()
    : shell(0)
{
}

WaylandIntegration *WaylandIntegration::instance()
{
    return s_waylandIntegration();
}

void WaylandIntegration::handleGlobal(void *data,
                                      struct wl_registry *registry,
                                      uint32_t id,
                                      const char *interface,
                                      uint32_t version)
{
    Q_UNUSED(version);

    qDebug() << "Wayland interface:" << interface;

    if (strcmp(interface, "desktop_shell") == 0) {
        WaylandIntegration *object = static_cast<WaylandIntegration *>(data);
        Q_ASSERT(object);

        object->shell = static_cast<struct desktop_shell *>(
                    wl_registry_bind(registry, id, &desktop_shell_interface, 1));
        desktop_shell_add_listener(object->shell, &listener, data);

        // Platform native interface
        QPlatformNativeInterface *native =
                QGuiApplication::platformNativeInterface();
        Q_ASSERT(native);

        DesktopShell *shell = DesktopShell::instance();

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
            output->background = new Background(screen, shell);
            output->backgroundSurface = static_cast<struct wl_surface *>(
                        native->nativeResourceForWindow("surface",
                                                        output->background->window()));
            desktop_shell_set_background(object->shell, output->output,
                                         output->backgroundSurface);
            qDebug() << "Created background surface" << output->backgroundSurface
                     << "for output" << output->output;

            // Create a launcher window for each output
            output->launcher = new Launcher(screen, shell);
            output->launcherSurface = static_cast<struct wl_surface *>(
                        native->nativeResourceForWindow("surface",
                                                        output->launcher->window()));
            desktop_shell_set_panel(object->shell, output->output,
                                    output->launcherSurface);
            qDebug() << "Created launcher surface" << output->launcherSurface
                     << "for output" << output->output;

            shell->addOutput(output);
        }
    }
}

void WaylandIntegration::handleConfigure(void *data,
                                         struct desktop_shell *desktop_shell,
                                         uint32_t edges,
                                         struct wl_surface *surface,
                                         int32_t width, int32_t height)
{
    Q_UNUSED(desktop_shell);
    Q_UNUSED(edges);
    Q_UNUSED(width);
    Q_UNUSED(height);

    qDebug() << "Configure received for surface" << surface;

    WaylandIntegration *object = static_cast<WaylandIntegration *>(data);
    Q_ASSERT(object);

    DesktopShell *shell = DesktopShell::instance();

    foreach (Output *output, shell->outputs()) {
        if (output->backgroundSurface == surface) {
            output->background->window()->show();

            qDebug() << "Background geometry"
                     << output->background->window()->geometry()
                     << "for screen" << output->screen->name();
        } else if (output->launcherSurface == surface) {
            output->launcher->window()->show();

            qDebug() << "Launcher geometry"
                     << output->launcher->window()->geometry()
                     << "for screen" << output->screen->name();
        }
    }
}

void WaylandIntegration::handlePrepareLockSurface(void *data,
                                                  struct desktop_shell *desktop_shell)
{
    desktop_shell_unlock(desktop_shell);
}

void WaylandIntegration::handleGrabCursor(void *data,
                                          struct desktop_shell *desktop_shell,
                                          uint32_t cursor)
{
}
