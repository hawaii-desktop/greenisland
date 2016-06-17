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

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "windowmanager.h"
#include "windowmanager_p.h"

namespace GreenIsland {

namespace Server {

/*
 * WindowManagerPrivate
 */

QQmlListProperty<ClientWindow> WindowManagerPrivate::windows()
{
    Q_Q(WindowManager);
    return QQmlListProperty<ClientWindow>(q, Q_NULLPTR, windowsCount, windowsAt);
}

void WindowManagerPrivate::registerWindow(ClientWindow *window)
{
    Q_Q(WindowManager);

    if (!windowsList.contains(window)) {
        windowsList.append(window);
        Q_EMIT q->windowCreated(window);
    }
}

void WindowManagerPrivate::unregisterWindow(ClientWindow *window)
{
    Q_Q(WindowManager);

    if (windowsList.removeOne(window))
        Q_EMIT q->windowClosed(window);
}

void WindowManagerPrivate::recalculateVirtualGeometry()
{
    QRect geometry;

    Q_FOREACH (QWaylandOutput *output, compositor->outputs())
        geometry = geometry.united(output->geometry());

    rootItem->setPosition(geometry.topLeft());
    rootItem->setSize(geometry.size());
}

void WindowManagerPrivate::_q_outputAdded(QWaylandOutput *)
{
    recalculateVirtualGeometry();
}

void WindowManagerPrivate::_q_outputRemoved(QWaylandOutput *)
{
    recalculateVirtualGeometry();
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
}

WindowManager::WindowManager(QWaylandCompositor *compositor, QObject *parent)
    : QObject(*new WindowManagerPrivate(), parent)
{
    Q_D(WindowManager);
    d->compositor = compositor;
}

QWaylandCompositor *WindowManager::compositor() const
{
    Q_D(const WindowManager);
    return d->compositor;
}

void WindowManager::setCompositor(QWaylandCompositor *compositor)
{
    Q_D(WindowManager);

    if (d->compositor != compositor)
        return;

    if (d->compositor) {
        disconnect(d->compositor, SIGNAL(outputAdded(QWaylandOutput*)),
                   this, SLOT(_q_outputAdded(QWaylandOutput*)));
        disconnect(d->compositor, SIGNAL(outputRemoved(QWaylandOutput*)),
                   this, SLOT(_q_outputRemoved(QWaylandOutput*)));
    }

    if (compositor) {
        connect(compositor, SIGNAL(outputAdded(QWaylandOutput*)),
                this, SLOT(_q_outputAdded(QWaylandOutput*)));
        connect(compositor, SIGNAL(outputRemoved(QWaylandOutput*)),
                this, SLOT(_q_outputRemoved(QWaylandOutput*)));
    }

    d->compositor = compositor;
    Q_EMIT compositorChanged();
}

ClientWindow *WindowManager::createWindow(QWaylandSurface *surface)
{
    Q_D(WindowManager);

    // Create a new client window
    ClientWindow *clientWindow = new ClientWindow(this, surface);
    ClientWindowPrivate::get(clientWindow)->moveItem->setParentItem(d->rootItem);

    // Append to the list
    d->registerWindow(clientWindow);

    // Automatically delete client window when the surface is destroyed
    connect(surface, SIGNAL(surfaceDestroyed()),
            clientWindow, SLOT(deleteLater()));

    return clientWindow;
}

ClientWindow *WindowManager::windowForSurface(QWaylandSurface *surface) const
{
    Q_D(const WindowManager);

    if (surface) {
        Q_FOREACH (ClientWindow *window, d->windowsList) {
            if (window->surface() == surface)
                return window;
        }
    }

    return Q_NULLPTR;
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

} // namespace Server

} // namespace GreenIsland

#include "moc_windowmanager.cpp"
