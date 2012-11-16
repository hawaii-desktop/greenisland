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

#include "desktopshellintegration.h"

DesktopShellIntegration *DesktopShellIntegration::m_instance = 0;

DesktopShellIntegration::DesktopShellIntegration()
    : shell(0)
{
    m_instance = this;
}

DesktopShellIntegration *DesktopShellIntegration::createInstance()
{
    if (m_instance)
        return 0;
    return new DesktopShellIntegration();
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
    if (strcmp(interface, "desktop_shell") == 0 && version == 1) {
        DesktopShellIntegration *object = static_cast<DesktopShellIntegration *>(data);
        object->shell = static_cast<struct desktop_shell *>(
                            wl_registry_bind(registry, id, &desktop_shell_interface, 1));
    }
}
