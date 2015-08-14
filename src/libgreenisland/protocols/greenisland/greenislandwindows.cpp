/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "applicationmanager.h"
#include "clientwindow.h"
#include "greenislandwindow.h"
#include "greenislandwindows.h"

namespace GreenIsland {

GreenIslandWindows::GreenIslandWindows()
    : m_boundResource(Q_NULLPTR)
{
    ApplicationManager *appMan = ApplicationManager::instance();

    QObject::connect(appMan, &ApplicationManager::windowMapped, [this](ClientWindow *window) {
        if (!m_boundResource)
            return;

        // Only top level windows
        if (window->type() != ClientWindow::TopLevel)
            return;

        GreenIslandWindow *w = new GreenIslandWindow(m_boundResource->client(), window);
        m_windows.append(w);
        send_window_mapped(m_boundResource->handle, w->resource()->handle,
                           GreenIslandWindow::type2WlType(window->type()),
                           w->state(), window->title(), window->appId());
    });
    QObject::connect(appMan, &ApplicationManager::windowUnmapped, [this](ClientWindow *window) {
        if (!m_boundResource)
            return;

        Q_FOREACH (GreenIslandWindow *w, m_windows) {
            if (w->window() == window) {
                m_windows.removeOne(w);
                delete w;
                return;
            }
        }
    });
}

GreenIslandWindows::~GreenIslandWindows()
{
    while (!m_windows.isEmpty())
        delete m_windows.takeFirst();
}

const wl_interface *GreenIslandWindows::interface() const
{
    return &greenisland_windows_interface;
}

void GreenIslandWindows::bind(wl_client *client, uint32_t version, uint32_t id)
{
    m_boundResource = add(client, id, version);
}

void GreenIslandWindows::windows_bind_resource(Resource *resource)
{
    if (m_boundResource) {
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "greenisland_windows can only be bound once");
        return;
    }
}

}
