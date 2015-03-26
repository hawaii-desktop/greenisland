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

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandSurface>

#include "applicationmanager.h"
#include "applicationmanager_p.h"

namespace GreenIsland {

static ApplicationManager *s_applicationManager = Q_NULLPTR;

/*
 * ApplicationManagerPrivate
 */

ApplicationManagerPrivate::ApplicationManagerPrivate(ApplicationManager *parent)
    : q_ptr(parent)
{
}

void ApplicationManagerPrivate::registerSurface(QWaylandSurface *surface, const QString &appId)
{
    Q_Q(ApplicationManager);

    // Only top level windows qualify as applications
    if (surface->windowType() != QWaylandSurface::Toplevel)
        return;

    // In case the client goes away quickly
    if (!surface->client())
        return;

    pid_t pid = (pid_t)surface->client()->processId();

    if (appPids.contains(appId) && appPids[appId].contains(pid))
        return;
    if (appSurfaces.contains(surface))
        return;

    apps.insert(appId);
    appSurfaces[surface] = appId;
    appPids[appId].insert(pid);
    surfacePids[surface] = pid;

    Q_EMIT q->applicationAdded(appId, pid);
}

void ApplicationManagerPrivate::unregisterSurface(QWaylandSurface *surface, const QString &appId)
{
    Q_Q(ApplicationManager);

    // We can't access surface->client() here because it was already
    // deleted along with the surface
    pid_t pid = surfacePids[surface];

    if (appSurfaces.remove(surface) > 0) {
        // We might have more surfaces with the same pid
        bool found = false;
        Q_FOREACH (QWaylandSurface *s, appSurfaces.keys()) {
            if (surfacePids[s] == pid) {
                found = true;
                break;
            }
        }

        // If any of the remaining surfaces have this pid then we can remove
        if (!found)
            appPids.remove(appId);

        // Remove surface from surface pids hash
        surfacePids.remove(surface);

        // Deregister the application when it has no surfaces associated at all
        if (appSurfaces.count(surface) == 0) {
            apps.remove(appId);
            Q_EMIT q->applicationRemoved(appId, pid);
        }
    }
}

/*
 * ApplicationManager
 */

ApplicationManager::ApplicationManager()
    : QObject()
    , d_ptr(new ApplicationManagerPrivate(this))
{
    qRegisterMetaType<ApplicationManager *>("ApplicationManager*");

    s_applicationManager = this;
}

ApplicationManager::~ApplicationManager()
{
    delete d_ptr;
}

ApplicationManager *ApplicationManager::instance()
{
    if (!s_applicationManager)
        return new ApplicationManager();
    return s_applicationManager;
}

bool ApplicationManager::isRegistered(const QString &appId) const
{
    Q_D(const ApplicationManager);
    return d->apps.contains(appId);
}

void ApplicationManager::quit(const QString &appId)
{
    Q_D(ApplicationManager);

    Q_FOREACH (pid_t pid, d->appPids[appId]) {
        if (pid == QCoreApplication::applicationPid())
            continue;

        ::kill(pid, SIGTERM);
        QTimer::singleShot(5000, [pid] {
            ::kill(pid, SIGTERM);
        });
    }
}

}

#include "moc_applicationmanager.cpp"
