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

#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>

#include "waylandintegration.h"
#include "desktopshell.h"
#include "launcher.h"
#include "background.h"

WaylandIntegration *WaylandIntegration::m_instance = 0;

const struct wl_registry_listener WaylandIntegration::registryListener = {
    WaylandIntegration::handleGlobal
};

const struct desktop_shell_listener WaylandIntegration::listener = {
    WaylandIntegration::handleConfigure,
    WaylandIntegration::handlePrepareLockSurface,
    WaylandIntegration::handleGrabCursor
};

WaylandIntegration::WaylandIntegration(DesktopShell *shell)
    : shell(0)
    , protocolSync(0)
    , m_shell(shell)
{
    m_instance = this;
}

WaylandIntegration *WaylandIntegration::createInstance(DesktopShell *shell)
{
    if (m_instance)
        return 0;
    return new WaylandIntegration(shell);
}

WaylandIntegration *WaylandIntegration::instance()
{
    return m_instance;
}

void WaylandIntegration::handleGlobal(void *data,
                                      struct wl_registry *registry,
                                      uint32_t id,
                                      const char *interface,
                                      uint32_t version)
{
    Q_UNUSED(version);

    qDebug() << "Wayland interface:" << interface;

    if (strcmp(interface, "wl_output") == 0) {
        WaylandIntegration *object = static_cast<WaylandIntegration *>(data);
        Q_ASSERT(object);

        object->protocolSync++;
    }

    if (strcmp(interface, "desktop_shell") == 0) {
        WaylandIntegration *object = static_cast<WaylandIntegration *>(data);
        Q_ASSERT(object);

        object->shell = static_cast<struct desktop_shell *>(
                            wl_registry_bind(registry, id, &desktop_shell_interface, 1));
        desktop_shell_add_listener(object->shell, &listener, data);
        object->protocolSync++;
    }
}

void WaylandIntegration::handleConfigure(void *data,
                                         struct desktop_shell *desktop_shell,
                                         uint32_t edges,
                                         struct wl_surface *surface,
                                         int32_t width, int32_t height)
{
    WaylandIntegration *object = static_cast<WaylandIntegration *>(data);
    Q_ASSERT(object);

    DesktopShell *shell = DesktopShell::instance();

    foreach (Output *output, shell->outputs()) {
        if (output->backgroundSurface == surface) {
            QRect geometry(0, 0, width, height);
            qDebug() << "<============= Background" << geometry;
            output->background->window()->setGeometry(geometry);
            output->background->window()->show();
        }

#if 0
        if (output->launcherSurface == surface) {
            QRect geometry(0, 0, width, output->launcher->tileSize());
            qDebug() << "<============= Launcher" << geometry;
            output->launcher->setGeometry(geometry);
            output->launcher->show();
        }
#endif
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
