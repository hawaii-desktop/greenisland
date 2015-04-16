/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "globalregistry.h"
#include "logging.h"
#include "protocols/fullscreen-shell/fullscreenshellclient.h"

#include <wayland-client.h>
#include <wayland-server.h>

namespace GreenIsland {

Q_GLOBAL_STATIC(GlobalRegistry, s_globalRegistry)

GlobalRegistry::GlobalRegistry()
    : m_registry(Q_NULLPTR)
    , m_fullScreenShell(Q_NULLPTR)
{
}

GlobalRegistry::~GlobalRegistry()
{
    delete m_fullScreenShell;
}

void GlobalRegistry::start()
{
    QPlatformNativeInterface *native =
            QGuiApplication::platformNativeInterface();
    if (!native)
        qFatal("Platform native interface not found, aborting...");

    wl_display *display = static_cast<wl_display *>(
                native->nativeResourceForIntegration("display"));
    if (!display)
        qFatal("Wayland connection is not available, aborting...");

    m_registry = wl_display_get_registry(display);
    if (!m_registry)
        qFatal("Wayland registry unavailable, aborting...");
    wl_registry_add_listener(m_registry, &GlobalRegistry::m_listener, this);
    QCoreApplication::processEvents();
}

GlobalRegistry *GlobalRegistry::instance()
{
    return s_globalRegistry();
}

wl_registry *GlobalRegistry::registry()
{
    return s_globalRegistry()->m_registry;
}

FullScreenShellClient *GlobalRegistry::fullScreenShell()
{
    return s_globalRegistry()->m_fullScreenShell;
}

void GlobalRegistry::globalCreate(void *data, wl_registry *registry,
                                  uint32_t id, const char *interface,
                                  uint32_t version)
{
    Q_UNUSED(registry);

    GlobalRegistry *self = static_cast<GlobalRegistry *>(data);
    if (!self) {
        qCWarning(GREENISLAND_COMPOSITOR) << "Invalid data pointer passed to global interface creation!";
        return;
    }

    if (strcmp(interface, "_wl_fullscreen_shell") == 0 && version == 1)
        self->m_fullScreenShell = new FullScreenShellClient(id);
}

void GlobalRegistry::globalRemove(void *data, wl_registry *registry,
                                  uint32_t name)
{
    Q_UNUSED(registry);

    GlobalRegistry *self = static_cast<GlobalRegistry *>(data);
    if (!self) {
        qCWarning(GREENISLAND_COMPOSITOR) << "Invalid data pointer passed to global interface creation!";
        return;
    }

    if (self->m_fullScreenShell && self->m_fullScreenShell->id() == name) {
        delete self->m_fullScreenShell;
        self->m_fullScreenShell = Q_NULLPTR;
    }
}

const struct wl_registry_listener GlobalRegistry::m_listener = {
    GlobalRegistry::globalCreate,
    GlobalRegistry::globalRemove
};

}
