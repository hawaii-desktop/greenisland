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

#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickShellSurfaceItem>

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "clientwindowview.h"
#include "clientwindowview_p.h"
#include "serverlogging_p.h"
#include "windowmanager.h"
#include "windowmanager_p.h"
#include "extensions/quickxdgsurfaceitem.h"
#include "extensions/quickxdgpopupitem.h"

namespace GreenIsland {

namespace Server {

/*
 * ClientWindowPrivate
 */

QPointF ClientWindowPrivate::randomPosition() const
{
    // Surface size
    const int w = surface->size().width();
    const int h = surface->size().height();

    // Find the output where the pointer is located, defaults
    // to the default output
    // TODO: Need something clever for touch?
    QPoint pos = QCursor::pos();
    QWaylandOutput *output = surface->compositor()->defaultOutput();
    Q_FOREACH (QWaylandOutput *curOutput, surface->compositor()->outputs()) {
        if (curOutput->geometry().contains(pos)) {
            output = curOutput;
            break;
        }
    }

    // Output size
    const int ow = output->availableGeometry().size().width();
    const int oh = output->availableGeometry().size().height();

    const int step = 24;
    static int px = step;
    static int py = 2 * step;
    int dx, dy;

    // Increment new coordinates by the step
    px += step;
    py += 2 * step;
    if (px > ow / 2)
        px = step;
    if (py > oh / 2)
        py = step;
    dx = px;
    dy = py;
    if (dx + w > ow) {
        dx = ow - w;
        if (dx < 0)
            dx = 0;
        px = 0;
    }
    if (dy + h > oh) {
        dy = oh - h;
        if (dy < 0)
            dy = 0;
        py = 0;
    }

    return QPointF(dx, dy);
}

void ClientWindowPrivate::findOutputs()
{
    Q_Q(ClientWindow);

    if (!surface)
        return;

    const QRect geometry(moveItem->position().toPoint(), surface->size());
    int maxArea = 0, area = 0;
    QWaylandOutput *mainOutput = designedOutput;
    QList<QWaylandOutput *> list;

    // Find the outputs where this window is in
    Q_FOREACH (QWaylandOutput *output, surface->compositor()->outputs()) {
        // Append the outputs where the window is in
        if (output->geometry().contains(geometry))
            list.append(output);

        // Find the output that has the contains the biggest part of this window
        QRect intersected = output->geometry().intersected(geometry);
        if (intersected.isValid()) {
            area = intersected.width() * intersected.height();
            if (area >= maxArea) {
                mainOutput = output;
                maxArea = area;
            }
        }
    }

    if (list != outputsList) {
        outputsList = list;
        Q_EMIT q->outputsChanged();
    }

    if (mainOutput != designedOutput) {
        designedOutput = mainOutput;
        Q_EMIT q->designedOutputChanged();
    }
}

void ClientWindowPrivate::setSurface(QWaylandSurface *surface)
{
    if (this->surface) {
        qCWarning(gLcCore, "Setting ClientWindow surface is allowed only once");
        return;
    }

    this->surface = surface;
}

void ClientWindowPrivate::setType(ClientWindow::Type type)
{
    Q_Q(ClientWindow);

    if (this->type == type)
        return;

    this->type = type;
    Q_EMIT q->typeChanged();
}

void ClientWindowPrivate::setParentWindow(ClientWindow *window)
{
    Q_Q(ClientWindow);

    if (parentWindow == window)
        return;

    parentWindow = window;
    Q_EMIT q->parentWindowChanged();
}

void ClientWindowPrivate::setTitle(const QString &title)
{
    Q_Q(ClientWindow);

    if (this->title == title)
        return;

    this->title = title;
    Q_EMIT q->titleChanged();
}

void ClientWindowPrivate::setAppId(const QString &appId)
{
    Q_Q(ClientWindow);

    QString newAppId = appId;

    if (newAppId.isEmpty()) {
        // Use process name if appId is empty (some applications won't set it, like weston-terminal)
        QFile file(QStringLiteral("/proc/%1/cmdline").arg(surface->client()->processId()));
        if (file.open(QIODevice::ReadOnly)) {
            QFileInfo fi(QString::fromUtf8(file.readAll().split('\0').at(0)));
            newAppId = fi.baseName();
            file.close();
        }
    }

    if (this->appId == newAppId)
        return;

    this->appId = newAppId;
    Q_EMIT q->appIdChanged();

    // Icon name
    const QString &desktopFileName = findDesktopFile(this->appId);
    if (!desktopFileName.isEmpty()) {
        QSettings desktopEntry(desktopFileName, QSettings::IniFormat);
        desktopEntry.setIniCodec("UTF-8");
        desktopEntry.beginGroup(QStringLiteral("Desktop Entry"));
        const QString icon =
                desktopEntry.value(QStringLiteral("Icon"),
                                   QStringLiteral("application-octet-stream")).toString();
        if (this->iconName != icon) {
            this->iconName = icon;
            Q_EMIT q->iconNameChanged();
        }
    }
}

void ClientWindowPrivate::setActive(bool active)
{
    Q_Q(ClientWindow);

    if (this->active == active)
        return;

    this->active = active;
    Q_EMIT q->activeChanged();

    XdgSurface *shellSurface = XdgSurface::findIn(surface);
    if (shellSurface)
        shellSurface->setActive(active);

    if (active) {
        // Raise parent
        if (type == ClientWindow::Transient && parentWindow)
            parentWindow->raise();

        // Raise this window
        q->raise();
    }
}

void ClientWindowPrivate::setWindowGeometry(const QRect &geometry)
{
    Q_Q(ClientWindow);

    if (windowGeometry == geometry)
        return;

    windowGeometry = geometry;
    Q_EMIT q->windowGeometryChanged();
}

void ClientWindowPrivate::setMaximized(QWaylandOutput *output)
{
    Q_Q(ClientWindow);

    if (maximized)
        return;

    maximized = true;
    Q_EMIT q->maximizedChanged();

    savedMaximized = QPointF(x, y);

    QWaylandOutput *designedOutput = output ? output : surface->compositor()->defaultOutput();
    moveItem->setX(designedOutput->position().x());
    moveItem->setY(designedOutput->position().y());
}

void ClientWindowPrivate::unsetMaximized()
{
    Q_Q(ClientWindow);

    if (!maximized)
        return;

    maximized = false;
    Q_EMIT q->maximizedChanged();

    moveItem->setX(savedMaximized.x());
    moveItem->setY(savedMaximized.y());
}

void ClientWindowPrivate::setFullScreen(QWaylandOutput *output)
{
    Q_Q(ClientWindow);

    if (fullScreen)
        return;

    fullScreen = true;
    Q_EMIT q->fullScreenChanged();

    savedFullScreen = QPointF(x, y);

    QWaylandOutput *designedOutput = output ? output : surface->compositor()->defaultOutput();
    moveItem->setX(designedOutput->position().x());
    moveItem->setY(designedOutput->position().y());
}

void ClientWindowPrivate::unsetFullScreen()
{
    Q_Q(ClientWindow);

    if (!fullScreen)
        return;

    fullScreen = false;
    Q_EMIT q->fullScreenChanged();

    moveItem->setX(savedFullScreen.x());
    moveItem->setY(savedFullScreen.y());
}

void ClientWindowPrivate::setTopLevel()
{
    Q_Q(ClientWindow);

    setType(ClientWindow::TopLevel);
    setParentWindow(Q_NULLPTR);
    unsetMaximized();
    unsetFullScreen();

    QPointF pos = randomPosition();
    moveItem->setX(pos.x());
    moveItem->setY(pos.y());

    q->setActive(true);
}

void ClientWindowPrivate::setTransient(ClientWindow *parentWindow,
                                       const QPoint &relativeToParent,
                                       bool keyboardFocus)
{
    Q_Q(ClientWindow);

    setType(ClientWindow::Transient);
    setParentWindow(parentWindow);

    moveItem->setX(parentWindow->x() + relativeToParent.x());
    moveItem->setY(parentWindow->y() + relativeToParent.y());

    q->setActive(keyboardFocus);
}

void ClientWindowPrivate::setPopup(QWaylandInputDevice *inputDevice,
                                   QWaylandSurface *parentSurface,
                                   const QPoint &relativeToParent)
{
    Q_UNUSED(inputDevice);

    setType(ClientWindow::Popup);

    ClientWindow *parentWindow =
            WindowManagerPrivate::get(wm)->windowForSurface(parentSurface);
    Q_ASSERT(parentWindow);
    setParentWindow(parentWindow);
    ClientWindowPrivate *dParentWindow = ClientWindowPrivate::get(parentWindow);
    Q_ASSERT(dParentWindow);

    Q_FOREACH (ClientWindowView *view, views) {
        Q_FOREACH (ClientWindowView *parentView, dParentWindow->views) {
            if (parentView->output() == view->output()) {
                view->setParentItem(parentView);
                view->setPosition(relativeToParent);
                break;
            }
        }
    }
}

QString ClientWindowPrivate::findDesktopFile(const QString &appId)
{
    return QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                  QStringLiteral("%1.desktop").arg(appId));
}

QQmlListProperty<ClientWindowView> ClientWindowPrivate::windowViews()
{
    Q_Q(ClientWindow);
    return QQmlListProperty<ClientWindowView>(q, Q_NULLPTR, windowViewsCount, windowViewsAt);
}

int ClientWindowPrivate::windowViewsCount(QQmlListProperty<ClientWindowView> *prop)
{
    ClientWindow *that = static_cast<ClientWindow *>(prop->object);
    return ClientWindowPrivate::get(that)->views.count();
}

ClientWindowView *ClientWindowPrivate::windowViewsAt(QQmlListProperty<ClientWindowView> *prop, int index)
{
    ClientWindow *that = static_cast<ClientWindow *>(prop->object);
    return ClientWindowPrivate::get(that)->views.at(index);
}

QQmlListProperty<QWaylandOutput> ClientWindowPrivate::outputs()
{
    Q_Q(ClientWindow);
    return QQmlListProperty<QWaylandOutput>(q, Q_NULLPTR, outputsCount, outputsAt);
}

int ClientWindowPrivate::outputsCount(QQmlListProperty<QWaylandOutput> *prop)
{
    ClientWindow *that = static_cast<ClientWindow *>(prop->object);
    return ClientWindowPrivate::get(that)->outputsList.count();
}

QWaylandOutput *ClientWindowPrivate::outputsAt(QQmlListProperty<QWaylandOutput> *prop, int index)
{
    ClientWindow *that = static_cast<ClientWindow *>(prop->object);
    return ClientWindowPrivate::get(that)->outputsList.at(index);
}

/*
 * ClientWindow
 */

ClientWindow::ClientWindow(QWaylandSurface *surface, QObject *parent)
    : QObject(*new ClientWindowPrivate(), parent)
{
    Q_D(ClientWindow);
    d->surface = surface;
    d->pid = surface->client()->processId();

    // Lock the buffer when the surface is being destroyed
    // so QtQuick has a chance to animate destruction
    connect(surface, &QWaylandSurface::surfaceDestroyed, this, [this, d] {
        Q_FOREACH (ClientWindowView *view, d->views)
            view->shellSurfaceItem()->view()->setBufferLock(true);
    });

    // Move
    connect(d->moveItem, &QQuickItem::xChanged, this, [this, d] {
        setX(d->moveItem->x());
    });
    connect(d->moveItem, &QQuickItem::yChanged, this, [this, d] {
        setY(d->moveItem->y());
    });
}

ClientWindow::~ClientWindow()
{
    Q_D(ClientWindow);

    // Activate the parent window
    if (d->type == Transient && d->parentWindow)
        ClientWindowPrivate::get(d->parentWindow)->setActive(true);
}

QWaylandSurface *ClientWindow::surface() const
{
    Q_D(const ClientWindow);
    return d->surface;
}

ClientWindow::Type ClientWindow::type() const
{
    Q_D(const ClientWindow);
    return d->type;
}

ClientWindow *ClientWindow::parentWindow() const
{
    Q_D(const ClientWindow);
    return d->parentWindow;
}

QString ClientWindow::title() const
{
    Q_D(const ClientWindow);
    return d->title;
}

QString ClientWindow::appId() const
{
    Q_D(const ClientWindow);
    return d->appId;
}

qint64 ClientWindow::processId() const
{
    Q_D(const ClientWindow);
    return d->pid;
}

QString ClientWindow::iconName() const
{
    Q_D(const ClientWindow);
    return d->iconName;
}

qreal ClientWindow::x() const
{
    Q_D(const ClientWindow);
    return d->x;
}

void ClientWindow::setX(qreal x)
{
    Q_D(ClientWindow);

    if (d->x == x)
        return;

    Q_FOREACH (ClientWindowView *view, d->views) {
        qreal localX = x - view->output()->geometry().left();
        view->setX(localX);
    }

    d->x = x;
    Q_EMIT xChanged();

    d->findOutputs();
}

qreal ClientWindow::y() const
{
    Q_D(const ClientWindow);
    return d->y;
}

void ClientWindow::setY(qreal y)
{
    Q_D(ClientWindow);

    if (d->y == y)
        return;

    Q_FOREACH (ClientWindowView *view, d->views) {
        qreal localY = y - view->output()->geometry().top();
        view->setY(localY);
    }

    d->y = y;
    Q_EMIT yChanged();

    d->findOutputs();
}

QPointF ClientWindow::position() const
{
    return QPointF(x(), y());
}

void ClientWindow::setPosition(const QPointF &pos)
{
    setX(pos.x());
    setY(pos.y());
}

QRect ClientWindow::windowGeometry() const
{
    Q_D(const ClientWindow);
    return d->windowGeometry;
}

QRect ClientWindow::taskIconGeometry() const
{
    Q_D(const ClientWindow);
    return d->taskIconGeometry;
}

void ClientWindow::setTaskIconGeometry(const QRect &rect)
{
    Q_D(ClientWindow);

    if (d->taskIconGeometry == rect)
        return;

    d->taskIconGeometry = rect;
    Q_EMIT taskIconGeometryChanged();
}

bool ClientWindow::isActive() const
{
    Q_D(const ClientWindow);
    return d->active;
}

void ClientWindow::setActive(bool active)
{
    Q_D(ClientWindow);

    WindowManagerPrivate *dWM = WindowManagerPrivate::get(d->wm);

    // Set focus on all windows, this will make the view call the
    // private version of setActive which will set the active flag
    // and raise the window
    Q_FOREACH (ClientWindow *window, dWM->windowsList) {
        // Give or take focus to the views of this window
        Q_FOREACH (ClientWindowView *view, ClientWindowPrivate::get(window)->views)
            view->shellSurfaceItem()->setFocus(active && window == this);
    }

    // Set keyboard focus to the surface of this window
    if (active)
        d->surface->compositor()->defaultInputDevice()->keyboard()->setFocus(d->surface);
}

bool ClientWindow::isMinimized() const
{
    Q_D(const ClientWindow);
    return d->minimized;
}

void ClientWindow::setMinimized(bool minimized)
{
    Q_D(ClientWindow);

    if (d->minimized == minimized)
        return;

    Q_FOREACH (ClientWindowView *view, d->views) {
        // No input events for minimized windows to prevent accidental interaction
        // with it (like keyboard input being sent to the window)
        if (view->shellSurfaceItem())
            view->shellSurfaceItem()->setInputEventsEnabled(!minimized);

        // Also, save the position if the window is going to be minimized
        if (minimized && view->savedProperties()) {
            view->savedProperties()->insert(QStringLiteral("x"), view->x());
            view->savedProperties()->insert(QStringLiteral("y"), view->y());
        }
    }

    d->minimized = minimized;
    Q_EMIT minimizedChanged();
}

bool ClientWindow::isMaximized() const
{
    Q_D(const ClientWindow);
    return d->maximized;
}

bool ClientWindow::isFullScreen() const
{
    Q_D(const ClientWindow);
    return d->fullScreen;
}

QWaylandOutput *ClientWindow::designedOutput() const
{
    Q_D(const ClientWindow);
    return d->designedOutput;
}

void ClientWindow::lower()
{
    Q_D(ClientWindow);

    Q_FOREACH (ClientWindowView *view, d->views) {
        QQuickItem *parent = view->parentItem();
        Q_ASSERT(parent);
        QQuickItem *bottom = parent->childItems().first();
        if (bottom != view)
            view->stackBefore(bottom);
    }
}

void ClientWindow::raise()
{
    Q_D(ClientWindow);

    Q_FOREACH (ClientWindowView *view, d->views) {
        QQuickItem *parent = view->parentItem();
        Q_ASSERT(parent);
        QQuickItem *top = parent->childItems().last();
        if (top != view)
            view->stackAfter(top);
    }
}

void ClientWindow::move(const QPoint &position)
{
    Q_D(ClientWindow);
    d->moveItem->setX(position.x());
    d->moveItem->setY(position.y());
}

} // namespace Server

} // namespace GreenIsland

#include "moc_clientwindow.cpp"
