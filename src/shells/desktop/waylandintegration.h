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

#ifndef WAYLANDINTEGRATION_H
#define WAYLANDINTEGRATION_H

#include "wayland-desktop-extension-client-protocol.h"

class DesktopShell;
class Launcher;
class Background;

class Output
{
public:
    struct wl_output *output;
    struct wl_list link;

    Launcher *launcher;
    struct wl_surface *launcherSurface;

    Background *background;
    struct wl_surface *backgroundSurface;
};

class WaylandIntegration
{
public:
    explicit WaylandIntegration(DesktopShell *shell);

    static WaylandIntegration *createInstance(DesktopShell *shell);

    static WaylandIntegration *instance();

    struct desktop_shell *shell;
    struct wl_list outputs;

    static const struct wl_registry_listener registryListener;

    static void handleGlobal(void *data,
                             wl_registry *registry,
                             uint32_t id,
                             const char *interface,
                             uint32_t version);

    static const struct desktop_shell_listener listener;

    static void handleConfigure(void *data,
                                struct desktop_shell *desktop_shell,
                                uint32_t edges,
                                struct wl_surface *surface,
                                int32_t width, int32_t height);

    static void handlePrepareLockSurface(void *data,
                                         struct desktop_shell *desktop_shell);

    static void handleGrabCursor(void *data,
                                 struct desktop_shell *desktop_shell,
                                 uint32_t cursor);

private:
    static WaylandIntegration *m_instance;
    DesktopShell *m_shell;
};

#endif // WAYLANDINTEGRATION_H
