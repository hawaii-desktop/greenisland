/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <GreenIsland/QtWaylandCompositor/QWaylandClient>
#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>

#include "applicationmanager.h"
#include "applicationmanager_p.h"
#include "serverlogging_p.h"
#include "shell/clientwindow.h"

namespace GreenIsland {

namespace Server {

/*
 * ApplicationManagerPrivate
 */

ApplicationManagerPrivate::ApplicationManagerPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_applications()
{
}

void ApplicationManagerPrivate::registerWindow(ClientWindow *window)
{
    Q_Q(ApplicationManager);

    if (appIdMap.values().contains(window))
        return;

    if (!window->appId().isEmpty()) {
        // Announce the appId
        Q_EMIT q->applicationAdded(window->appId(), window->surface()->client()->processId());

        // Insert the new appId
        appIdMap.insert(window->appId(), window);
    }

    QObject::connect(window, SIGNAL(appIdChanged()),
                     q, SLOT(_q_appIdChanged()));
    QObject::connect(window, SIGNAL(activeChanged()),
                     q, SLOT(_q_activeChanged()));
}

void ApplicationManagerPrivate::unregisterWindow(ClientWindow *window)
{
    Q_Q(ApplicationManager);

    QObject::disconnect(window, SIGNAL(appIdChanged()),
                        q, SLOT(_q_appIdChanged()));
    QObject::disconnect(window, SIGNAL(activeChanged()),
                        q, SLOT(_q_activeChanged()));

    QStringList keys = appIdMap.keys();
    Q_FOREACH (const QString &curAppId, keys) {
        if (appIdMap.remove(curAppId, window) > 0) {
            if (appIdMap.count(curAppId) == 0)
                appIdMap.remove(curAppId);
        }
    }

    // Emit the signal if this was the last window with that appId
    if (!appIdMap.contains(window->appId()))
        Q_EMIT q->applicationRemoved(window->appId(), window->processId());
}

void ApplicationManagerPrivate::_q_appIdChanged()
{
    Q_Q(ApplicationManager);

    ClientWindow *window = qobject_cast<ClientWindow *>(q->sender());
    Q_ASSERT(window);

    // Deassociate window with the previous appId if it has changed
    // and remove empty appId entries
    QSet<QString> oldAppIds;
    QStringList keys = appIdMap.keys();
    Q_FOREACH (const QString &curAppId, keys) {
        if (appIdMap.remove(curAppId, window) > 0)
            oldAppIds.insert(curAppId);

        if (appIdMap.values(curAppId).count() == 0)
            appIdMap.remove(curAppId);
    }

    // Emit a signal for each old appId that was removed
    Q_FOREACH (const QString &curAppId, oldAppIds)
        Q_EMIT q->applicationRemoved(curAppId, window->processId());

    if (!window->appId().isEmpty()) {
        // Announce the appId
        Q_EMIT q->applicationAdded(window->appId(), window->processId());

        // Insert the new appId
        appIdMap.insert(window->appId(), window);
    }
}

void ApplicationManagerPrivate::_q_activeChanged()
{
    Q_Q(ApplicationManager);

    ClientWindow *window = qobject_cast<ClientWindow *>(q->sender());
    Q_ASSERT(window);

    if (!window->isActive())
        return;

    Q_EMIT q->applicationFocused(window->appId());

    Q_FOREACH (const QString &appId, appIdMap.keys()) {
        if (appId != window->appId())
            Q_EMIT q->applicationUnfocused(appId);
    }
}

void ApplicationManagerPrivate::applications_quit(Resource *resource, const QString &app_id)
{
    Q_Q(ApplicationManager);

    Q_UNUSED(resource);

    q->quit(app_id);
}

/*
 * ApplicationManager
 */

ApplicationManager::ApplicationManager()
    : QWaylandExtensionTemplate<ApplicationManager>(*new ApplicationManagerPrivate())
{
}

ApplicationManager::ApplicationManager(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<ApplicationManager>(compositor, *new ApplicationManagerPrivate())
{
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
        qCWarning(gLcCore,
                  "Quit requested on the unknown appId %s has no effect",
                  qPrintable(appId));
        return;
    }

    ClientWindow *window = d->appIdMap.values(appId).at(0);
    if (!window) {
        qCWarning(gLcCore,
                  "Quit requested on the unregistered appId %s has no effect",
                  qPrintable(appId));
        return;
    }

    if (!window->surface()) {
        qCWarning(gLcCore,
                  "Quit requested on appId %s has no effect because surface cannot be found",
                  qPrintable(appId));
        return;
    }
    window->surface()->client()->close();
}

void ApplicationManager::initialize()
{
    Q_D(ApplicationManager);

    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing ApplicationManager";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *ApplicationManager::interface()
{
    return ApplicationManagerPrivate::interface();
}

QByteArray ApplicationManager::interfaceName()
{
    return ApplicationManagerPrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_applicationmanager.cpp"
