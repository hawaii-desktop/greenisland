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

#ifndef GLOBALREGISTRY_H
#define GLOBALREGISTRY_H

struct wl_registry;
struct wl_registry_listener;

namespace GreenIsland {

class FullScreenShellClient;

class GlobalRegistry
{
public:
    GlobalRegistry();
    ~GlobalRegistry();

    void start();

    static GlobalRegistry *instance();

    static wl_registry *registry();
    static FullScreenShellClient *fullScreenShell();

private:
    wl_registry *m_registry;
    FullScreenShellClient *m_fullScreenShell;

    static void globalCreate(void *data, wl_registry *registry,
                             uint32_t id, const char *interface,
                             uint32_t version);
    static void globalRemove(void *data, wl_registry *registry,
                             uint32_t name);

    static const struct wl_registry_listener m_listener;
};

}

#endif // GLOBALREGISTRY_H
