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

#include <GreenIsland/QtWaylandCompositor/QWaylandClient>
#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>

#include "applicationmanager.h"
#include "applicationmanager_p.h"
#include "logging.h"

namespace GreenIsland {

namespace Server {

/*
 * ApplicationManagerPrivate
 */

void ApplicationManagerPrivate::_q_appIdChanged(const QString &appId)
{
    Q_Q(ApplicationManager);

    QObject *window = q->sender();

    // Deassociate window with the previous appId if it has changed
    // and remove empty appId entries
    QStringList keys = appIdMap.keys();
    Q_FOREACH (const QString &curAppId, keys) {
        appIdMap.remove(curAppId, window);
        if (appIdMap.values(curAppId).count() == 0)
            appIdMap.remove(curAppId);
    }

    // Get the surface
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(
                qvariant_cast<QObject *>(window->property("surface")));
    if (!surface)
        return;

    // Emit the signal if this is the first time
    if (!appIdMap.contains(appId) && surface)
        Q_EMIT q->applicationAdded(appId, surface->client()->processId());
}

/*
 * ApplicationManager
 */

ApplicationManager::ApplicationManager(QObject *parent)
    : QObject(*new ApplicationManagerPrivate(), parent)
{
}

void ApplicationManager::registerWindow(QObject *window)
{
    Q_D(ApplicationManager);

    if (d->appIdMap.values().contains(window))
        return;

    const QString appId = window->property("appId").toString();
    if (!appId.isEmpty())
        d->appIdMap.insert(appId, window);

    connect(window, SIGNAL(appIdChanged(QString)),
            this, SLOT(_q_appIdChanged(QString)));
}

void ApplicationManager::unregisterWindow(QObject *window)
{
    Q_D(ApplicationManager);

    disconnect(window, SIGNAL(appIdChanged(QString)),
               this, SLOT(_q_appIdChanged(QString)));

    QStringList keys = d->appIdMap.keys();
    Q_FOREACH (const QString &curAppId, keys) {
        if (d->appIdMap.values(curAppId).contains(window)) {
            d->appIdMap.remove(curAppId, window);
            if (d->appIdMap.count(curAppId) == 0)
                d->appIdMap.remove(curAppId);
        }
    }
}

bool ApplicationManager::isRegistered(const QString &appId) const
{
    Q_D(const ApplicationManager);
    return d->appIdMap.contains(appId);
}

void ApplicationManager::quit(const QString &appId)
{
    Q_D(ApplicationManager);

    if (!d->appIdMap.contains(appId)) {
        qCWarning(GREENISLAND_COMPOSITOR,
                  "Quit requested on the unknown appId %s has no effect",
                  qPrintable(appId));
        return;
    }

    QObject *window = d->appIdMap.values(appId).at(0);
    if (!window) {
        qCWarning(GREENISLAND_COMPOSITOR,
                  "Quit requested on the unregistered appId %s has no effect",
                  qPrintable(appId));
        return;
    }

    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(
                qvariant_cast<QObject *>(window->property("surface")));
    if (!surface) {
        qCWarning(GREENISLAND_COMPOSITOR,
                  "Quit requested on appId %s has no effect because surface cannot be found",
                  qPrintable(appId));
        return;
    }
    surface->client()->close();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_applicationmanager.cpp"
