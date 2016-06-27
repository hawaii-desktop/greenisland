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
#include "shell/clientwindow_p.h"

namespace GreenIsland {

namespace Server {

/*
 * ApplicationManagerPrivate
 */

ApplicationManagerPrivate::ApplicationManagerPrivate()
    : QWaylandCompositorExtensionPrivate()
    , QtWaylandServer::greenisland_applications()
    , rootItem(new QQuickItem())
{
    // Populate appId mapping
    appIdMap[QLatin1String("org.hawaiios.hawaii-system-preferences")] = QLatin1String("org.hawaiios.SystemPreferences");
    appIdMap[QLatin1String("org.hawaiios.hawaii-screenshot")] = QLatin1String("org.hawaiios.Screenshot");
    appIdMap[QLatin1String("org.hawaiios.hawaii-terminal")] = QLatin1String("org.hawaiios.terminal");
    appIdMap[QLatin1String("org.qterminal.qterminal-qt5")] = QLatin1String("qterminal-qt5");
}

ApplicationManagerPrivate::~ApplicationManagerPrivate()
{
    delete rootItem;
}

void ApplicationManagerPrivate::registerWindow(ClientWindow *window)
{
    Q_Q(ApplicationManager);

    if (windowsList.contains(window))
        return;

    // Append the window
    windowsList.append(window);
    Q_EMIT q->windowCreated(window);

    // Append to the applications list
    if (!window->appId().isEmpty() && !appIds.contains(window->appId())) {
        appIds.append(window->appId());
        Q_EMIT q->applicationAdded(window->appId(), window->processId());
    }

    QObject::connect(window, SIGNAL(appIdChanged()),
                     q, SLOT(_q_appIdChanged()));
    QObject::connect(window, SIGNAL(activatedChanged()),
                     q, SLOT(_q_activatedChanged()));
}

void ApplicationManagerPrivate::unregisterWindow(ClientWindow *window)
{
    Q_Q(ApplicationManager);

    // First remove the window to avoid checking appId against itself
    if (!windowsList.removeOne(window)) {
        qCWarning(gLcCore) << "Couldn't unregister window" << window;
        return;
    }

    // Disconnect the window
    QObject::disconnect(window, SIGNAL(appIdChanged()),
                        q, SLOT(_q_appIdChanged()));
    QObject::disconnect(window, SIGNAL(activatedChanged()),
                        q, SLOT(_q_activatedChanged()));

    // Go through the windows list and if there is no window with the
    // same appId left it means the application was closed
    Q_FOREACH (ClientWindow *curWindow, windowsList) {
        if (curWindow->appId() == window->appId())
            return;
    }

    // Unregister the application
    appIds.removeOne(window->appId());
    Q_EMIT q->applicationRemoved(window->appId(), window->processId());
}

void ApplicationManagerPrivate::recalculateVirtualGeometry()
{
    QRect geometry;

    Q_FOREACH (QWaylandOutput *output, compositor->outputs())
        geometry = geometry.united(output->geometry());

    rootItem->setPosition(geometry.topLeft());
    rootItem->setSize(geometry.size());
}

void ApplicationManagerPrivate::_q_outputAdded(QWaylandOutput *)
{
    recalculateVirtualGeometry();
}

void ApplicationManagerPrivate::_q_outputRemoved(QWaylandOutput *)
{
    recalculateVirtualGeometry();
}

void ApplicationManagerPrivate::_q_appIdChanged()
{
    Q_Q(ApplicationManager);

    ClientWindow *window = qobject_cast<ClientWindow *>(q->sender());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);

    // Unregister the old appId
    if (appIds.contains(dWindow->prevAppId)) {
        appIds.removeOne(dWindow->prevAppId);
        Q_EMIT q->applicationRemoved(dWindow->prevAppId, window->processId());
    }

    // Register the new appId
    if (!appIds.contains(window->appId())) {
        appIds.append(window->appId());
        Q_EMIT q->applicationAdded(window->appId(), window->processId());
    }
}

void ApplicationManagerPrivate::_q_activatedChanged()
{
    Q_Q(ApplicationManager);

    ClientWindow *window = qobject_cast<ClientWindow *>(q->sender());
    Q_ASSERT(window);

    if (!window->activated())
        return;

    Q_EMIT q->applicationFocused(window->appId());
}

QQmlListProperty<ClientWindow> ApplicationManagerPrivate::windows()
{
    Q_Q(ApplicationManager);
    return QQmlListProperty<ClientWindow>(q, Q_NULLPTR, windowsCount, windowsAt);
}

int ApplicationManagerPrivate::windowsCount(QQmlListProperty<ClientWindow> *prop)
{
    ApplicationManager *that = static_cast<ApplicationManager *>(prop->object);
    return ApplicationManagerPrivate::get(that)->windowsList.count();
}

ClientWindow *ApplicationManagerPrivate::windowsAt(QQmlListProperty<ClientWindow> *prop, int index)
{
    ApplicationManager *that = static_cast<ApplicationManager *>(prop->object);
    return ApplicationManagerPrivate::get(that)->windowsList.at(index);
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
    : QWaylandCompositorExtensionTemplate<ApplicationManager>(*new ApplicationManagerPrivate())
{
}

ApplicationManager::ApplicationManager(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<ApplicationManager>(compositor, *new ApplicationManagerPrivate())
{
}

QWaylandCompositor *ApplicationManager::compositor() const
{
    Q_D(const ApplicationManager);
    return d->compositor;
}

ClientWindow *ApplicationManager::createWindow(QWaylandSurface *surface)
{
    Q_D(ApplicationManager);

    // Create a new client window
    ClientWindow *clientWindow = new ClientWindow(this, surface);
    ClientWindowPrivate::get(clientWindow)->moveItem->setParentItem(d->rootItem);

    // Append to the list
    d->registerWindow(clientWindow);

    // Automatically delete client window when the surface is destroyed
    connect(surface, &QWaylandSurface::surfaceDestroyed, this, [this, d, clientWindow] {
        // Unregister window
        d->unregisterWindow(clientWindow);

        // Activate the parent window
        if (clientWindow->type() == ClientWindow::Transient && clientWindow->parentWindow()) {
            auto views = ClientWindowPrivate::get(clientWindow->parentWindow())->views;
            Q_FOREACH (QWaylandQuickItem *view, views)
                view->takeFocus();
        }

        // Delete window
        clientWindow->deleteLater();
    });

    return clientWindow;
}

ClientWindow *ApplicationManager::windowForSurface(QWaylandSurface *surface) const
{
    Q_D(const ApplicationManager);

    if (surface) {
        Q_FOREACH (ClientWindow *window, d->windowsList) {
            if (window->surface() == surface)
                return window;
        }
    }

    return Q_NULLPTR;
}

QVariantList ApplicationManager::windowsForOutput(QWaylandOutput *desiredOutput) const
{
    Q_D(const ApplicationManager);

    if (!d->compositor)
        return QVariantList();

    QVariantList list;
    QWaylandOutput *output = desiredOutput ? desiredOutput : d->compositor->defaultOutput();

    Q_FOREACH (ClientWindow *window, d->windowsList) {
        if (window->designedOutput() == output)
            list.append(QVariant::fromValue(window));
    }

    return list;
}

bool ApplicationManager::isRegistered(const QString &appId) const
{
    Q_D(const ApplicationManager);
    return d->appIds.contains(appId);
}

void ApplicationManager::quit(const QString &appId)
{
    Q_D(ApplicationManager);

    // Cannot quit an unkown application
    if (!d->appIds.contains(appId)) {
        qCWarning(gLcCore,
                  "Quit requested on unregistered appId %s has no effect",
                  qPrintable(appId));
        return;
    }

    // Close the client
    Q_FOREACH (ClientWindow *curWindow, d->windowsList) {
        if (curWindow->appId() == appId) {
            curWindow->surface()->client()->close();
            return;
        }
    }
}

void ApplicationManager::initialize()
{
    Q_D(ApplicationManager);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing ApplicationManager";
        return;
    }
    d->compositor = compositor;
    d->init(compositor->display(), 1);

    Q_EMIT compositorChanged();

    connect(compositor, SIGNAL(outputAdded(QWaylandOutput*)),
            this, SLOT(_q_outputAdded(QWaylandOutput*)));
    connect(compositor, SIGNAL(outputRemoved(QWaylandOutput*)),
            this, SLOT(_q_outputRemoved(QWaylandOutput*)));
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
