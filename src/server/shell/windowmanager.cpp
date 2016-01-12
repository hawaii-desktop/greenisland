/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "serverlogging_p.h"
#include "windowmanager.h"
#include "windowmanager_p.h"
#include "core/applicationmanager.h"
#include "core/applicationmanager_p.h"

namespace GreenIsland {

namespace Server {

/*
 * WindowManagerPrivate
 */

ClientWindow *WindowManagerPrivate::windowForSurface(QWaylandSurface *surface) const
{
    Q_FOREACH (ClientWindow *window, windowsList) {
        if (window->surface() == surface)
            return window;
    }

    return Q_NULLPTR;
}

QQmlListProperty<ClientWindow> WindowManagerPrivate::windows()
{
    Q_Q(WindowManager);
    return QQmlListProperty<ClientWindow>(q, Q_NULLPTR, windowsCount, windowsAt);
}

int WindowManagerPrivate::windowsCount(QQmlListProperty<ClientWindow> *prop)
{
    WindowManager *that = static_cast<WindowManager *>(prop->object);
    return WindowManagerPrivate::get(that)->windowsList.count();
}

ClientWindow *WindowManagerPrivate::windowsAt(QQmlListProperty<ClientWindow> *prop, int index)
{
    WindowManager *that = static_cast<WindowManager *>(prop->object);
    return WindowManagerPrivate::get(that)->windowsList.at(index);
}

/*
 * WindowManager
 */

WindowManager::WindowManager(QObject *parent)
    : QObject(*new WindowManagerPrivate(), parent)
{
    Q_D(WindowManager);
    d->wlShell = new QWaylandShell();
    d->xdgShell = new XdgShell();
    d->gtkShell = new GtkShell();
}

WindowManager::WindowManager(QWaylandCompositor *compositor, QObject *parent)
    : QObject(*new WindowManagerPrivate(), parent)
{
    Q_D(WindowManager);
    d->compositor = compositor;
    d->wlShell = new QWaylandShell(compositor);
    d->xdgShell = new XdgShell(compositor);
    d->gtkShell = new GtkShell(compositor);
}

QWaylandCompositor *WindowManager::compositor() const
{
    Q_D(const WindowManager);
    return d->compositor;
}

void WindowManager::setCompositor(QWaylandCompositor *compositor)
{
    Q_D(WindowManager);

    if (d->initialized) {
        qCWarning(gLcCore,
                  "Setting QWaylandCompositor %p on WindowManager %p "
                  "is not supported after WindowManager has been initialized.",
                  compositor, this);
        return;
    }

    if (d->compositor && d->compositor != compositor)
        qCWarning(gLcCore,
                  "Possible initialization error. Moving WindowManager %p between compositor instances.",
                  this);

    d->compositor = compositor;

    d->wlShell->setExtensionContainer(compositor);
    d->xdgShell->setExtensionContainer(compositor);
    d->gtkShell->setExtensionContainer(compositor);

    Q_EMIT compositorChanged();
}

QVariantList WindowManager::windowsForOutput(QWaylandOutput *desiredOutput) const
{
    Q_D(const WindowManager);

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

void WindowManager::recalculateVirtualGeometry()
{
    Q_D(WindowManager);

    if (!d->compositor || !d->initialized)
        return;

    QRect geometry;

    Q_FOREACH (QWaylandOutput *output, d->compositor->outputs())
        geometry = geometry.united(output->geometry());

    d->rootItem->setPosition(geometry.topLeft());
    d->rootItem->setSize(geometry.size());
}

void WindowManager::initialize()
{
    Q_D(WindowManager);

    if (d->initialized)
        return;

    connect(d->wlShell, &QWaylandShell::createShellSurface,
            this, &WindowManager::createWlShellSurface);
    connect(d->xdgShell, &XdgShell::createSurface,
            this, &WindowManager::createXdgSurface);
    connect(d->xdgShell, &XdgShell::createPopup,
            this, &WindowManager::createXdgPopup);
    connect(d->gtkShell, &GtkShell::createSurface,
            this, &WindowManager::createGtkShellSurface);

    d->wlShell->initialize();
    d->xdgShell->initialize();
    d->gtkShell->initialize();

    d->appMan = ApplicationManager::findIn(d->compositor);

    d->initialized = true;

    recalculateVirtualGeometry();
}

void WindowManager::createWlShellSurface(QWaylandSurface *surface,
                                         QWaylandClient *client,
                                         uint id)
{
    Q_D(WindowManager);

    QWaylandShellSurface *shellSurface =
            new QWaylandShellSurface(d->wlShell, surface, client, id);

    ClientWindow *window = new ClientWindow(surface, this);
    d->windowsList.append(window);
    Q_EMIT windowsChanged();

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    dWindow->wm = this;
    dWindow->interfaceName = QWaylandShellSurface::interfaceName();
    dWindow->moveItem->setParentItem(d->rootItem);
    dWindow->setAppId(QString());

    connect(surface, &QWaylandSurface::surfaceDestroyed,
            this, &WindowManager::surfaceDestroyed);

    if (!d->appMan)
        d->appMan = ApplicationManager::findIn(d->compositor);
    if (d->appMan)
        ApplicationManagerPrivate::get(d->appMan)->registerWindow(window);

    Q_EMIT windowCreated(window);

    connect(shellSurface, &QWaylandShellSurface::classNameChanged,
            this, &WindowManager::setAppId);
    connect(shellSurface, &QWaylandShellSurface::titleChanged,
            this, &WindowManager::setTitle);
    connect(shellSurface, &QWaylandShellSurface::setDefaultToplevel,
            this, &WindowManager::setTopLevel);
    connect(shellSurface, &QWaylandShellSurface::setTransient,
            this, &WindowManager::setWlTransient);
    connect(shellSurface, &QWaylandShellSurface::setPopup,
            this, &WindowManager::setWlPopup);
    connect(shellSurface, &QWaylandShellSurface::setMaximized,
            this, &WindowManager::setMaximized);
    connect(shellSurface, &QWaylandShellSurface::setFullScreen,
            this, &WindowManager::setWlFullScreen);
    connect(shellSurface, &QWaylandShellSurface::pong,
            window, &ClientWindow::pong);
}

void WindowManager::createXdgSurface(QWaylandSurface *surface,
                                     QWaylandClient *client,
                                     uint id)
{
    Q_D(WindowManager);

    XdgSurface *shellSurface =
            new XdgSurface(d->xdgShell, surface, client, id);

    ClientWindow *window = new ClientWindow(surface, this);
    d->windowsList.append(window);
    Q_EMIT windowsChanged();

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    dWindow->wm = this;
    dWindow->interfaceName = XdgSurface::interfaceName();
    dWindow->moveItem->setParentItem(d->rootItem);
    dWindow->setAppId(QString());

    connect(surface, &QWaylandSurface::surfaceDestroyed,
            this, &WindowManager::surfaceDestroyed);

    if (!d->appMan)
        d->appMan = ApplicationManager::findIn(d->compositor);
    if (d->appMan)
        ApplicationManagerPrivate::get(d->appMan)->registerWindow(window);

    Q_EMIT windowCreated(window);

    connect(shellSurface, &XdgSurface::appIdChanged,
            this, &WindowManager::setAppId);
    connect(shellSurface, &XdgSurface::titleChanged,
            this, &WindowManager::setTitle);
    connect(shellSurface, &XdgSurface::windowGeometryChanged,
            this, &WindowManager::setXdgWindowGeometry);
    connect(shellSurface, &XdgSurface::setDefaultToplevel,
            this, &WindowManager::setTopLevel);
    connect(shellSurface, &XdgSurface::setTransient,
            this, &WindowManager::setXdgTransient);
    connect(shellSurface, &XdgSurface::maximizedChanged,
            this, &WindowManager::setMaximized);
    connect(shellSurface, &XdgSurface::fullScreenChanged,
            this, &WindowManager::setXdgFullScreen);
    connect(shellSurface, &XdgSurface::pingRequested,
            window, &ClientWindow::pingRequested);
    connect(shellSurface, &XdgSurface::pong,
            window, &ClientWindow::pong);
    connect(shellSurface, &XdgSurface::windowMenuRequested,
            window, &ClientWindow::windowMenuRequested);
}

void WindowManager::createXdgPopup(QWaylandInputDevice *inputDevice,
                                   QWaylandSurface *surface,
                                   QWaylandSurface *parentSurface,
                                   const QPoint &relativeToParent,
                                   QWaylandClient *client, uint id)
{
    Q_D(WindowManager);

    new XdgPopup(d->xdgShell, inputDevice, surface, parentSurface, client, id);

    ClientWindow *window = new ClientWindow(surface, this);
    d->windowsList.append(window);
    Q_EMIT windowsChanged();

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    dWindow->wm = this;
    dWindow->interfaceName = XdgPopup::interfaceName();
    dWindow->setAppId(QString());

    connect(surface, &QWaylandSurface::surfaceDestroyed,
            this, &WindowManager::surfaceDestroyed);

    Q_EMIT windowCreated(window);

    dWindow->setPopup(inputDevice, parentSurface, relativeToParent);
}

void WindowManager::createGtkShellSurface(QWaylandSurface *surface,
                                          QWaylandClient *client,
                                          uint id)
{
    Q_D(WindowManager);

    GtkSurface *gtkSurface =
            new GtkSurface(d->gtkShell, surface, client, id);
    connect(gtkSurface, &GtkSurface::appIdChanged, this,
            [this, d, surface](const QString &appId) {
        if (!appId.isEmpty()) {
            ClientWindow *window = d->windowForSurface(surface);
            if (window)
                ClientWindowPrivate::get(window)->setAppId(appId);
        }
    });
}

void WindowManager::setAppId()
{
    Q_D(WindowManager);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandShellSurface *wlShellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    if (wlShellSurface)
        surface = wlShellSurface->surface();

    XdgSurface *xdgShellSurface = qobject_cast<XdgSurface *>(sender());
    if (xdgShellSurface)
        surface = xdgShellSurface->surface();

    ClientWindow *window = d->windowForSurface(surface);
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    if (wlShellSurface)
        dWindow->setAppId(wlShellSurface->className());

    if (xdgShellSurface)
        dWindow->setAppId(xdgShellSurface->appId());
}

void WindowManager::setTitle()
{
    Q_D(WindowManager);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandShellSurface *wlShellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    if (wlShellSurface)
        surface = wlShellSurface->surface();

    XdgSurface *xdgShellSurface = qobject_cast<XdgSurface *>(sender());
    if (xdgShellSurface)
        surface = xdgShellSurface->surface();

    ClientWindow *window = d->windowForSurface(surface);
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    if (wlShellSurface)
        dWindow->setTitle(wlShellSurface->title());

    if (xdgShellSurface)
        dWindow->setTitle(xdgShellSurface->title());
}

void WindowManager::setXdgWindowGeometry()
{
    Q_D(WindowManager);

    XdgSurface *shellSurface = qobject_cast<XdgSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setWindowGeometry(shellSurface->windowGeometry());
}

void WindowManager::setTopLevel()
{
    Q_D(WindowManager);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandShellSurface *wlShellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    if (wlShellSurface)
        surface = wlShellSurface->surface();

    XdgSurface *xdgShellSurface = qobject_cast<XdgSurface *>(sender());
    if (xdgShellSurface)
        surface = xdgShellSurface->surface();

    ClientWindow *window = d->windowForSurface(surface);
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setTopLevel();
}

void WindowManager::setWlTransient(QWaylandSurface *parentSurface,
                                   const QPoint &relativeToParent,
                                   QWaylandShellSurface::FocusPolicy policy)
{
    Q_D(WindowManager);

    QWaylandShellSurface *shellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    ClientWindow *parentWindow = d->windowForSurface(parentSurface);
    Q_ASSERT(parentWindow);

    QPoint pos = relativeToParent;
    if (pos.isNull()) {
        pos.setX((parentSurface->size().width() - shellSurface->surface()->size().width()) / 2);
        pos.setY((parentSurface->size().height() - shellSurface->surface()->size().height()) / 2);
    }

    dWindow->setTransient(parentWindow, pos,
                          policy == QWaylandShellSurface::DefaultFocus);
}

void WindowManager::setWlPopup(QWaylandInputDevice *inputDevice,
                               QWaylandSurface *parentSurface,
                               const QPoint &relativeToParent)
{
    Q_D(WindowManager);

    QWaylandShellSurface *shellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setPopup(inputDevice, parentSurface, relativeToParent);
}

void WindowManager::setXdgTransient(QWaylandSurface *parentSurface)
{
    Q_D(WindowManager);

    XdgSurface *shellSurface = qobject_cast<XdgSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    ClientWindow *parentWindow = d->windowForSurface(parentSurface);
    Q_ASSERT(parentWindow);

    QPoint pos;
    pos.setX((parentSurface->size().width() - shellSurface->surface()->size().width()) / 2);
    pos.setY((parentSurface->size().height() - shellSurface->surface()->size().height()) / 2);

    dWindow->setTransient(parentWindow, pos, true);
}

void WindowManager::setMaximized(QWaylandOutput *output)
{
    Q_D(WindowManager);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandShellSurface *wlShellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    if (wlShellSurface)
        surface = wlShellSurface->surface();

    XdgSurface *xdgShellSurface = qobject_cast<XdgSurface *>(sender());
    if (xdgShellSurface)
        surface = xdgShellSurface->surface();

    ClientWindow *window = d->windowForSurface(surface);
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setMaximized(output);
}

void WindowManager::setWlFullScreen(QWaylandShellSurface::FullScreenMethod,
                                    uint, QWaylandOutput *output)
{
    Q_D(WindowManager);

    QWaylandShellSurface *shellSurface = qobject_cast<QWaylandShellSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setFullScreen(output);
}

void WindowManager::setXdgFullScreen(QWaylandOutput *output)
{
    Q_D(WindowManager);

    XdgSurface *shellSurface = qobject_cast<XdgSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setFullScreen(output);
}

void WindowManager::surfaceDestroyed()
{
    Q_D(WindowManager);

    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

    ClientWindow *window = d->windowForSurface(surface);
    if (!window)
        return;

    if (d->appMan)
        ApplicationManagerPrivate::get(d->appMan)->unregisterWindow(window);

    // We don't have to disconnect the signals here because both surface
    // and shell surface are already gone here

    // Remove the window
    d->windowsList.removeOne(window);
    window->deleteLater();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_windowmanager.cpp"
