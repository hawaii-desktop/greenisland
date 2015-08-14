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
#include "greenislandapps.h"

namespace GreenIsland {

GreenIslandApps::GreenIslandApps()
    : m_boundResource(Q_NULLPTR)
{
    ApplicationManager *appMan = ApplicationManager::instance();

    QObject::connect(appMan, &ApplicationManager::applicationAdded, [this](const QString &appId, pid_t pid) {
        if (m_boundResource)
            send_registered(m_boundResource->handle, appId, pid);
    });
    QObject::connect(appMan, &ApplicationManager::applicationRemoved, [this](const QString &appId, pid_t pid) {
        if (m_boundResource)
            send_unregistered(m_boundResource->handle, appId, pid);
    });
    QObject::connect(appMan, &ApplicationManager::applicationFocused, [this](const QString &appId) {
        if (m_boundResource)
            send_focused(m_boundResource->handle, appId);
    });
    QObject::connect(appMan, &ApplicationManager::applicationUnfocused, [this](const QString &appId) {
        if (m_boundResource)
            send_unfocused(m_boundResource->handle, appId);
    });
}

const wl_interface *GreenIslandApps::interface() const
{
    return &greenisland_applications_interface;
}

void GreenIslandApps::bind(wl_client *client, uint32_t version, uint32_t id)
{
    m_boundResource = add(client, id, version);
}

void GreenIslandApps::applications_bind_resource(Resource *resource)
{
    if (m_boundResource) {
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "greenisland_applications can be bound only once");
        return;
    }
}

void GreenIslandApps::applications_quit(Resource *resource, const QString &appId)
{
    Q_UNUSED(resource)
    ApplicationManager::instance()->quit(appId);
}

}
