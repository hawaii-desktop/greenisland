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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "serverlogging_p.h"
#include "unifiedshell.h"
#include "unifiedshell_p.h"
#include "core/applicationmanager.h"
#include "core/applicationmanager_p.h"

namespace GreenIsland {

namespace Server {

/*
 * UnifiedShellPrivate
 */

ClientWindow *UnifiedShellPrivate::windowForSurface(QWaylandSurface *surface) const
{
    Q_FOREACH (ClientWindow *window, windowsList) {
        if (window->surface() == surface)
            return window;
    }

    return Q_NULLPTR;
}

QQmlListProperty<ClientWindow> UnifiedShellPrivate::windows()
{
    Q_Q(UnifiedShell);
    return QQmlListProperty<ClientWindow>(q, Q_NULLPTR, windowsCount, windowsAt);
}

int UnifiedShellPrivate::windowsCount(QQmlListProperty<ClientWindow> *prop)
{
    UnifiedShell *that = static_cast<UnifiedShell *>(prop->object);
    return UnifiedShellPrivate::get(that)->windowsList.count();
}

ClientWindow *UnifiedShellPrivate::windowsAt(QQmlListProperty<ClientWindow> *prop, int index)
{
    UnifiedShell *that = static_cast<UnifiedShell *>(prop->object);
    return UnifiedShellPrivate::get(that)->windowsList.at(index);
}

/*
 * UnifiedShell
 */

UnifiedShell::UnifiedShell(QObject *parent)
    : QObject(*new UnifiedShellPrivate(), parent)
{
    Q_D(UnifiedShell);
    d->wlShell = new QWaylandWlShell();
    d->xdgShell = new XdgShell();
    d->gtkShell = new GtkShell();
}

UnifiedShell::UnifiedShell(QWaylandCompositor *compositor, QObject *parent)
    : QObject(*new UnifiedShellPrivate(), parent)
{
    Q_D(UnifiedShell);
    d->compositor = compositor;
    d->wlShell = new QWaylandWlShell(compositor);
    d->xdgShell = new XdgShell(compositor);
    d->gtkShell = new GtkShell(compositor);
}

QWaylandCompositor *UnifiedShell::compositor() const
{
    Q_D(const UnifiedShell);
    return d->compositor;
}

void UnifiedShell::setCompositor(QWaylandCompositor *compositor)
{
    Q_D(UnifiedShell);

    if (d->initialized) {
        qCWarning(gLcCore,
                  "Setting QWaylandCompositor %p on UnifiedShell %p "
                  "is not supported after UnifiedShell has been initialized.",
                  compositor, this);
        return;
    }

    if (d->compositor && d->compositor != compositor)
        qCWarning(gLcCore,
                  "Possible initialization error. Moving UnifiedShell %p between compositor instances.",
                  this);

    d->compositor = compositor;

    d->wlShell->setExtensionContainer(compositor);
    d->xdgShell->setExtensionContainer(compositor);
    d->gtkShell->setExtensionContainer(compositor);

    Q_EMIT compositorChanged();
}

QVariantList UnifiedShell::windowsForOutput(QWaylandOutput *desiredOutput) const
{
    Q_D(const UnifiedShell);

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

void UnifiedShell::recalculateVirtualGeometry()
{
    Q_D(UnifiedShell);

    if (!d->compositor || !d->initialized)
        return;

    QRect geometry;

    Q_FOREACH (QWaylandOutput *output, d->compositor->outputs())
        geometry = geometry.united(output->geometry());

    d->rootItem->setPosition(geometry.topLeft());
    d->rootItem->setSize(geometry.size());
}

void UnifiedShell::initialize()
{
    Q_D(UnifiedShell);

    if (d->initialized)
        return;

    connect(d->wlShell, &QWaylandWlShell::createShellSurface,
            this, &UnifiedShell::createWlShellSurface);
    connect(d->xdgShell, &XdgShell::createSurface,
            this, &UnifiedShell::createXdgSurface);
    connect(d->xdgShell, &XdgShell::createPopup,
            this, &UnifiedShell::createXdgPopup);
    connect(d->gtkShell, &GtkShell::createSurface,
            this, &UnifiedShell::createGtkShellSurface);

    d->wlShell->initialize();
    d->xdgShell->initialize();
    d->gtkShell->initialize();

    d->appMan = ApplicationManager::findIn(d->compositor);

    d->initialized = true;

    recalculateVirtualGeometry();
}

void UnifiedShell::createWlShellSurface(QWaylandSurface *surface,
                                         const QWaylandResource &resource)
{
    Q_D(UnifiedShell);

    QWaylandWlShellSurface *shellSurface =
            new QWaylandWlShellSurface(d->wlShell, surface, resource);

    ClientWindow *window = new ClientWindow(surface, this);
    d->windowsList.append(window);
    Q_EMIT windowsChanged();

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    dWindow->wm = this;
    dWindow->interfaceName = QWaylandWlShellSurface::interfaceName();
    dWindow->moveItem->setParentItem(d->rootItem);
    dWindow->setAppId(QString());
    dWindow->setWindowGeometry(QRect(QPoint(0, 0), surface->size()));

    connect(surface, &QWaylandSurface::sizeChanged,
            this, &UnifiedShell::setWlWindowGeometry);
    connect(surface, &QWaylandSurface::surfaceDestroyed,
            this, &UnifiedShell::surfaceDestroyed);

    if (!d->appMan)
        d->appMan = ApplicationManager::findIn(d->compositor);
    if (d->appMan)
        ApplicationManagerPrivate::get(d->appMan)->registerWindow(window);

    Q_EMIT windowCreated(window);

    connect(shellSurface, &QWaylandWlShellSurface::classNameChanged,
            this, &UnifiedShell::setAppId);
    connect(shellSurface, &QWaylandWlShellSurface::titleChanged,
            this, &UnifiedShell::setTitle);
    connect(shellSurface, &QWaylandWlShellSurface::setDefaultToplevel,
            this, &UnifiedShell::setTopLevel);
    connect(shellSurface, &QWaylandWlShellSurface::setTransient,
            this, &UnifiedShell::setWlTransient);
    connect(shellSurface, &QWaylandWlShellSurface::setPopup,
            this, &UnifiedShell::setWlPopup);
    connect(shellSurface, &QWaylandWlShellSurface::setMaximized,
            this, &UnifiedShell::setMaximized);
    connect(shellSurface, &QWaylandWlShellSurface::setFullScreen,
            this, &UnifiedShell::setWlFullScreen);
    connect(shellSurface, &QWaylandWlShellSurface::pong,
            window, &ClientWindow::pong);
}

void UnifiedShell::createXdgSurface(QWaylandSurface *surface,
                                     const QWaylandResource &resource)
{
    Q_D(UnifiedShell);

    XdgSurface *shellSurface =
            new XdgSurface(d->xdgShell, surface, resource);

    ClientWindow *window = new ClientWindow(surface, this);
    d->windowsList.append(window);
    Q_EMIT windowsChanged();

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    dWindow->wm = this;
    dWindow->interfaceName = XdgSurface::interfaceName();
    dWindow->moveItem->setParentItem(d->rootItem);
    dWindow->setAppId(QString());

    connect(surface, &QWaylandSurface::surfaceDestroyed,
            this, &UnifiedShell::surfaceDestroyed);

    if (!d->appMan)
        d->appMan = ApplicationManager::findIn(d->compositor);
    if (d->appMan)
        ApplicationManagerPrivate::get(d->appMan)->registerWindow(window);

    Q_EMIT windowCreated(window);

    connect(shellSurface, &XdgSurface::appIdChanged,
            this, &UnifiedShell::setAppId);
    connect(shellSurface, &XdgSurface::titleChanged,
            this, &UnifiedShell::setTitle);
    connect(shellSurface, &XdgSurface::windowGeometryChanged,
            this, &UnifiedShell::setXdgWindowGeometry);
    connect(shellSurface, &XdgSurface::setDefaultToplevel,
            this, &UnifiedShell::setTopLevel);
    connect(shellSurface, &XdgSurface::setTransient,
            this, &UnifiedShell::setXdgTransient);
    connect(shellSurface, &XdgSurface::maximizedChanged,
            this, &UnifiedShell::setMaximized);
    connect(shellSurface, &XdgSurface::fullScreenChanged,
            this, &UnifiedShell::setXdgFullScreen);
    connect(shellSurface, &XdgSurface::pingRequested,
            window, &ClientWindow::pingRequested);
    connect(shellSurface, &XdgSurface::pong,
            window, &ClientWindow::pong);
    connect(shellSurface, &XdgSurface::windowMenuRequested,
            window, &ClientWindow::windowMenuRequested);
}

void UnifiedShell::createXdgPopup(QWaylandInputDevice *inputDevice,
                                   QWaylandSurface *surface,
                                   QWaylandSurface *parentSurface,
                                   const QPoint &relativeToParent,
                                   const QWaylandResource &resource)
{
    Q_D(UnifiedShell);

    new XdgPopup(d->xdgShell, inputDevice, surface, parentSurface, resource);

    ClientWindow *window = new ClientWindow(surface, this);
    d->windowsList.append(window);
    Q_EMIT windowsChanged();

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    dWindow->wm = this;
    dWindow->interfaceName = XdgPopup::interfaceName();
    dWindow->setAppId(QString());

    connect(surface, &QWaylandSurface::surfaceDestroyed,
            this, &UnifiedShell::surfaceDestroyed);

    Q_EMIT windowCreated(window);

    dWindow->setPopup(inputDevice, parentSurface, relativeToParent);
}

void UnifiedShell::createGtkShellSurface(QWaylandSurface *surface,
                                          QWaylandClient *client,
                                          uint id)
{
    Q_D(UnifiedShell);

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

void UnifiedShell::setAppId()
{
    Q_D(UnifiedShell);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
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

void UnifiedShell::setTitle()
{
    Q_D(UnifiedShell);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
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

void UnifiedShell::setWlWindowGeometry()
{
    Q_D(UnifiedShell);

    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    Q_ASSERT(surface);

    ClientWindow *window = d->windowForSurface(surface);
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setWindowGeometry(QRect(QPoint(0, 0), surface->size()));
}

void UnifiedShell::setXdgWindowGeometry()
{
    Q_D(UnifiedShell);

    XdgSurface *shellSurface = qobject_cast<XdgSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setWindowGeometry(shellSurface->windowGeometry());
}

void UnifiedShell::setTopLevel()
{
    Q_D(UnifiedShell);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
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

void UnifiedShell::setWlTransient(QWaylandSurface *parentSurface,
                                   const QPoint &relativeToParent,
                                   QWaylandWlShellSurface::FocusPolicy policy)
{
    Q_D(UnifiedShell);

    QWaylandWlShellSurface *shellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
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
                          policy == QWaylandWlShellSurface::DefaultFocus);
}

void UnifiedShell::setWlPopup(QWaylandInputDevice *inputDevice,
                               QWaylandSurface *parentSurface,
                               const QPoint &relativeToParent)
{
    Q_D(UnifiedShell);

    QWaylandWlShellSurface *shellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setPopup(inputDevice, parentSurface, relativeToParent);
}

void UnifiedShell::setXdgTransient(QWaylandSurface *parentSurface)
{
    Q_D(UnifiedShell);

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

void UnifiedShell::setMaximized(QWaylandOutput *output)
{
    Q_D(UnifiedShell);

    QWaylandSurface *surface = Q_NULLPTR;

    QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
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

void UnifiedShell::setWlFullScreen(QWaylandWlShellSurface::FullScreenMethod,
                                    uint, QWaylandOutput *output)
{
    Q_D(UnifiedShell);

    QWaylandWlShellSurface *shellSurface = qobject_cast<QWaylandWlShellSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setFullScreen(output);
}

void UnifiedShell::setXdgFullScreen(QWaylandOutput *output)
{
    Q_D(UnifiedShell);

    XdgSurface *shellSurface = qobject_cast<XdgSurface *>(sender());
    Q_ASSERT(shellSurface);

    ClientWindow *window = d->windowForSurface(shellSurface->surface());
    Q_ASSERT(window);
    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);
    Q_ASSERT(dWindow);

    dWindow->setFullScreen(output);
}

void UnifiedShell::surfaceDestroyed()
{
    Q_D(UnifiedShell);

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

#include "moc_unifiedshell.cpp"
