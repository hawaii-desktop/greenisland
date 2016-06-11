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

#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/QWaylandShellSurface>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "windowmanager_p.h"
#include "serverlogging_p.h"
#include "extensions/applicationmanager.h"
#include "extensions/applicationmanager_p.h"

namespace GreenIsland {

namespace Server {

/*
 * ClientWindowPrivate
 */

ClientWindowPrivate::ClientWindowPrivate()
    : windowManager(Q_NULLPTR)
    , applicationManager(Q_NULLPTR)
    , wlShell(Q_NULLPTR)
    , xdgShell(Q_NULLPTR)
    , gtkShell(Q_NULLPTR)
    , surface(Q_NULLPTR)
    , type(ClientWindow::Unknown)
    , parentWindow(Q_NULLPTR)
    , pid(0)
    , x(0)
    , y(0)
    , savedMaximized(QPointF(0, 0))
    , savedFullScreen(QPointF(0, 0))
    , taskIconGeometry(QRect(0, 0, 32, 32))
    , active(false)
    , minimized(false)
    , maximized(false)
    , fullscreen(false)
    , moveItem(new QQuickItem())
    , designedOutput(Q_NULLPTR)
{
}

void ClientWindowPrivate::initialize(QWaylandSurface *surface)
{
    Q_Q(ClientWindow);

    if (this->surface) {
        qCWarning(gLcCore, "ClientWindow is already initialized");
        return;
    }

    this->surface = surface;
    this->pid = surface->client()->processId();

    Q_EMIT q->surfaceChanged();
    Q_EMIT q->processIdChanged();

    // Assign a random position by default
    moveItem->setPosition(q->randomPosition());
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
    Q_EMIT q->activatedChanged();
}

void ClientWindowPrivate::setWindowGeometry(const QRect &geometry)
{
    Q_Q(ClientWindow);

    if (windowGeometry == geometry)
        return;

    windowGeometry = geometry;
    Q_EMIT q->windowGeometryChanged();
}

void ClientWindowPrivate::setMaximized(bool maximized)
{
    Q_Q(ClientWindow);

    if (this->maximized == maximized)
        return;

    this->maximized = maximized;
    Q_EMIT q->maximizedChanged();
}

void ClientWindowPrivate::setFullscreen(bool fullscreen)
{
    Q_Q(ClientWindow);

    if (this->fullscreen == fullscreen)
        return;

    this->fullscreen = fullscreen;
    Q_EMIT q->fullscreenChanged();
}

void ClientWindowPrivate::_q_wlSurfaceCreated(QWaylandWlShellSurface *wlShellSurface)
{
    Q_Q(ClientWindow);

    if (wlShellSurface->surface() != surface)
        return;

    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::titleChanged, q, [this, wlShellSurface] {
        setTitle(wlShellSurface->title());
    });
    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::classNameChanged, q, [this, wlShellSurface] {
        setAppId(wlShellSurface->className());
    });
    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::setDefaultToplevel, q, [this, wlShellSurface] {
        setParentWindow(Q_NULLPTR);
        setMaximized(false);
        setFullscreen(false);
        setType(ClientWindow::TopLevel);
    });
    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::setTransient, q, [this, wlShellSurface]
                     (QWaylandSurface *parentSurface, const QPoint &, bool) {
        setParentWindow(windowManager->windowForSurface(parentSurface));
        setMaximized(false);
        setFullscreen(false);
        setType(ClientWindow::Transient);
    });
    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::setFullScreen, q, [this, wlShellSurface]
                     (QWaylandWlShellSurface::FullScreenMethod, uint, QWaylandOutput *) {
        setFullscreen(true);
    });
    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::setPopup, q, [this, wlShellSurface]
                     (QWaylandInputDevice *, QWaylandSurface *parentSurface, const QPoint &) {
        setParentWindow(windowManager->windowForSurface(parentSurface));
        setMaximized(false);
        setType(ClientWindow::Popup);
    });
    QObject::connect(wlShellSurface, &QWaylandWlShellSurface::setMaximized, q, [this, wlShellSurface]
                     (QWaylandOutput *) {
        setMaximized(true);
        setFullscreen(false);
    });
}

void ClientWindowPrivate::_q_xdgSurfaceCreated(QWaylandXdgSurface *xdgSurface)
{
    Q_Q(ClientWindow);

    if (xdgSurface->surface() != surface)
        return;

    QObject::connect(xdgSurface, &QWaylandXdgSurface::setTopLevel, q, [this, q, xdgSurface] {
        setType(ClientWindow::TopLevel);
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::setTransient, q, [this, q, xdgSurface] {
        setParentWindow(windowManager->windowForSurface(xdgSurface->parentSurface()->surface()));

        // Set position relative to parent
        // FIXME: We don't know the surface size yet because the buffer is not yet committed
        if (parentWindow) {
            qreal startX = parentWindow->x() + (parentWindow->surface()->size().width() / 2);
            qreal startY = parentWindow->y() + (parentWindow->surface()->size().height() / 2);
            moveItem->setPosition(QPointF(startX - (xdgSurface->windowGeometry().size().width() / 2),
                                          startY - (xdgSurface->windowGeometry().size().height() / 2)));

            setType(ClientWindow::Transient);
        }
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::titleChanged, q, [this, xdgSurface] {
        setTitle(xdgSurface->title());
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::appIdChanged, q, [this, xdgSurface] {
        setAppId(xdgSurface->appId());
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::windowGeometryChanged, q, [this, xdgSurface] {
        setWindowGeometry(xdgSurface->windowGeometry());
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::maximizedChanged, q, [this, q, xdgSurface] {
        setMaximized(xdgSurface->maximized());
        q->setMinimized(false);
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::fullscreenChanged, q, [this, q, xdgSurface] {
        setFullscreen(xdgSurface->fullscreen());
        q->setMinimized(false);
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::activatedChanged, q, [this, q, xdgSurface] {
        setActive(xdgSurface->activated());
        q->setMinimized(false);

        if (active) {
            // Raise parent
            if (type == ClientWindow::Transient && parentWindow)
                parentWindow->raise();

            // Raise this window
            q->raise();
        }
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::setMinimized, q, [this, q, xdgSurface] {
        q->setMinimized(true);
    });
    QObject::connect(xdgSurface, &QWaylandXdgSurface::showWindowMenu,
                     q, &ClientWindow::showWindowMenu);
}

void ClientWindowPrivate::_q_gtkSurfaceCreated(GtkSurface *gtkSurface)
{
    Q_Q(ClientWindow);

    if (gtkSurface->surface() != surface)
        return;

    QObject::connect(gtkSurface, &GtkSurface::appIdChanged, q, [this](const QString &appId) {
        setAppId(appId);
    });
}

QString ClientWindowPrivate::findDesktopFile(const QString &appId)
{
    return QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                  QStringLiteral("%1.desktop").arg(appId));
}

QQmlListProperty<QWaylandQuickItem> ClientWindowPrivate::windowViews()
{
    Q_Q(ClientWindow);
    return QQmlListProperty<QWaylandQuickItem>(q, Q_NULLPTR, windowViewsCount, windowViewsAt);
}

int ClientWindowPrivate::windowViewsCount(QQmlListProperty<QWaylandQuickItem> *prop)
{
    ClientWindow *that = static_cast<ClientWindow *>(prop->object);
    return ClientWindowPrivate::get(that)->views.count();
}

QWaylandQuickItem *ClientWindowPrivate::windowViewsAt(QQmlListProperty<QWaylandQuickItem> *prop, int index)
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

ClientWindow::ClientWindow(WindowManager *wm, QWaylandSurface *surface)
    : QObject(*new ClientWindowPrivate(), wm)
{
    Q_D(ClientWindow);
    d->windowManager = wm;

    // Shells
    d->wlShell = QWaylandWlShell::findIn(surface->compositor());
    if (d->wlShell)
        connect(d->wlShell, SIGNAL(wlSurfaceCreated(QWaylandWlShellSurface*)),
                this, SLOT(_q_wlSurfaceCreated(QWaylandWlShellSurface*)));
    d->xdgShell = QWaylandXdgShell::findIn(surface->compositor());
    if (d->xdgShell)
        connect(d->xdgShell, SIGNAL(xdgSurfaceCreated(QWaylandXdgSurface*)),
                this, SLOT(_q_xdgSurfaceCreated(QWaylandXdgSurface*)));
    d->gtkShell = GtkShell::findIn(surface->compositor());
    if (d->gtkShell)
        connect(d->gtkShell, SIGNAL(gtkSurfaceCreated(GtkSurface*)),
                this, SLOT(_q_gtkSurfaceCreated(GtkSurface*)));

    // Move
    connect(d->moveItem, &QQuickItem::xChanged, this, [this, d] {
        setX(d->moveItem->x());
    });
    connect(d->moveItem, &QQuickItem::yChanged, this, [this, d] {
        setY(d->moveItem->y());
    });

    // Initialize
    d->initialize(surface);

    // Register window with the application manager
    d->applicationManager = ApplicationManager::findIn(surface->compositor());
    if (d->applicationManager)
        ApplicationManagerPrivate::get(d->applicationManager)->registerWindow(this);
}

ClientWindow::~ClientWindow()
{
    Q_D(ClientWindow);

    // Activate the parent window
    if (d->type == Transient && d->parentWindow) {
        auto views = ClientWindowPrivate::get(d->parentWindow)->views;
        Q_FOREACH (QWaylandQuickItem *view, views)
            view->takeFocus();
    }

    // Unregister window from the window manager
    WindowManagerPrivate::get(d->windowManager)->unregisterWindow(this);

    // Unregister window from the application manager
    if (d->applicationManager)
        ApplicationManagerPrivate::get(d->applicationManager)->unregisterWindow(this);
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

bool ClientWindow::activated() const
{
    Q_D(const ClientWindow);
    return d->active;
}

bool ClientWindow::minimized() const
{
    Q_D(const ClientWindow);
    return d->minimized;
}

void ClientWindow::setMinimized(bool minimized)
{
    Q_D(ClientWindow);

    if (d->minimized == minimized)
        return;

    /*
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
    */

    d->minimized = minimized;
    Q_EMIT minimizedChanged();
}

bool ClientWindow::maximized() const
{
    Q_D(const ClientWindow);
    return d->maximized;
}

bool ClientWindow::fullscreen() const
{
    Q_D(const ClientWindow);
    return d->fullscreen;
}

QQuickItem *ClientWindow::moveItem() const
{
    Q_D(const ClientWindow);
    return d->moveItem;
}

QWaylandOutput *ClientWindow::designedOutput() const
{
    Q_D(const ClientWindow);
    return d->designedOutput;
}

QPointF ClientWindow::randomPosition() const
{
    Q_D(const ClientWindow);

    // Surface size
    const int w = d->surface->size().width();
    const int h = d->surface->size().height();

    // Find the output where the pointer is located, defaults
    // to the default output
    // TODO: Need something clever for touch?
    QPoint pos = QCursor::pos();
    QWaylandOutput *output = d->surface->compositor()->defaultOutput();
    Q_FOREACH (QWaylandOutput *curOutput, d->surface->compositor()->outputs()) {
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

void ClientWindow::addWindowView(QWaylandQuickItem *item)
{
    Q_D(ClientWindow);

    if (!d->views.contains(item))
        d->views.append(item);
}

void ClientWindow::removeWindowView(QWaylandQuickItem *item)
{
    Q_D(ClientWindow);

    d->views.removeOne(item);
}

void ClientWindow::activate()
{
    Q_D(ClientWindow);

    Q_FOREACH (QWaylandQuickItem *view, d->views)
        view->takeFocus();
}

void ClientWindow::lower()
{
    Q_D(ClientWindow);

    Q_FOREACH (QWaylandQuickItem *view, d->views) {
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

    Q_FOREACH (QWaylandQuickItem *view, d->views) {
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

void ClientWindow::close()
{
    Q_D(ClientWindow);

    if (d->surface->role() == QWaylandXdgSurface::role())
        QWaylandXdgSurface::findIn(d->surface)->sendClose();
    else if (d->surface->role() == QWaylandXdgPopup::role())
        QWaylandXdgPopup::findIn(d->surface)->sendPopupDone();
    else
        d->surface->destroy();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_clientwindow.cpp"
