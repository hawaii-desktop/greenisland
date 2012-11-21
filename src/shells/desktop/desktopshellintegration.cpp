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
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>

#include "desktopshellintegration.h"
#include "desktopshell.h"
#include "shellview.h"

DesktopShellIntegration *DesktopShellIntegration::m_instance = 0;

DesktopShellIntegration::DesktopShellIntegration(DesktopShell *shell)
    : protocol(0)
    , m_shell(shell)
{
    m_instance = this;
}

DesktopShellIntegration *DesktopShellIntegration::createInstance(DesktopShell *shell)
{
    if (m_instance)
        return 0;
    return new DesktopShellIntegration(shell);
}

DesktopShellIntegration *DesktopShellIntegration::instance()
{
    return m_instance;
}

void DesktopShellIntegration::handleGlobal(void *data,
                                           struct wl_registry *registry,
                                           uint32_t id,
                                           const char *interface,
                                           uint32_t version)
{
    if (strcmp(interface, "desktop_shell") != 0 || version != 1)
        return;

    // Create the Wayland protocol object
    DesktopShellIntegration *object = static_cast<DesktopShellIntegration *>(data);
    object->protocol = static_cast<struct desktop_shell *>(
                           wl_registry_bind(registry, id, &desktop_shell_interface, 1));

    QPlatformNativeInterface *native =
        QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);

    // Pass surfave and geometry to the compositor
    struct wl_surface *surface =
        static_cast<struct wl_surface *>(
                native->nativeResourceForWindow("surface", object->m_shell->shellView()));
    Q_ASSERT(surface);
    desktop_shell_set_surface(object->protocol, surface);
    desktop_shell_set_geometry(object->protocol,
                               object->m_shell->shellView()->geometry().x(),
                               object->m_shell->shellView()->geometry().y(),
                               object->m_shell->shellView()->geometry().width(),
                               object->m_shell->shellView()->geometry().height());
}
