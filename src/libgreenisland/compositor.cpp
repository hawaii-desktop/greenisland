/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QProcess>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtCore/QVariantMap>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "applicationmanager_p.h"
#ifdef QT_COMPOSITOR_WAYLAND_GL
#  include "bufferattacher.h"
#endif
#include "cmakedirs.h"
#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "config.h"
#include "logging.h"
#include "windowview.h"
#include "shellwindow.h"

#include "protocols/greenisland/greenislandapps.h"
#include "protocols/greenisland/greenislandwindows.h"
#include "protocols/greenisland/greenislandrecorder.h"
#include "protocols/gtk-shell/gtkshell.h"
#include "protocols/plasma/plasmaeffects.h"
#include "protocols/plasma/plasmashell.h"
#include "protocols/wl-shell/wlshell.h"
#include "protocols/xdg-shell/xdgshell.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

namespace GreenIsland {

QString Compositor::s_fixedPlugin;

/*
 * CompositorPrivate
 */

CompositorPrivate::CompositorPrivate(Compositor *self)
    : running(false)
    , state(Compositor::Active)
    , idleInterval(5 * 60000)
    , idleInhibit(0)
    , locked(false)
    , cursorSurface(Q_NULLPTR)
    , cursorHotspotX(0)
    , cursorHotspotY(0)
    , lastKeyboardFocus(Q_NULLPTR)
    , recorderManager(Q_NULLPTR)
    , q_ptr(self)
{
    settings = new CompositorSettings(self);
    screenManager = new ScreenManager(self);
    appManager = new ApplicationManager(self);
}

QQmlListProperty<ClientWindow> CompositorPrivate::windows()
{
    Q_Q(Compositor);

    auto countFunc = [](QQmlListProperty<ClientWindow> *prop) {
        return static_cast<Compositor *>(prop->object)->d_func()->clientWindowsList.count();
    };
    auto atFunc = [](QQmlListProperty<ClientWindow> *prop, int index) {
        return static_cast<Compositor *>(prop->object)->d_func()->clientWindowsList.at(index);
    };
    return QQmlListProperty<ClientWindow>(q, 0, countFunc, atFunc);
}

QQmlListProperty<ShellWindow> CompositorPrivate::shellWindows()
{
    Q_Q(Compositor);

    auto countFunc = [](QQmlListProperty<ShellWindow> *prop) {
        return static_cast<Compositor *>(prop->object)->d_func()->shellWindowsList.count();
    };
    auto atFunc = [](QQmlListProperty<ShellWindow> *prop, int index) {
        return static_cast<Compositor *>(prop->object)->d_func()->shellWindowsList.at(index);
    };
    return QQmlListProperty<ShellWindow>(q, 0, countFunc, atFunc);
}

void CompositorPrivate::dpms(bool on)
{
    // TODO
    Q_UNUSED(on);
}

void CompositorPrivate::_q_updateCursor(bool hasBuffer)
{
    if (!hasBuffer || !cursorSurface || !cursorSurface->bufferAttacher())
        return;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QImage image = static_cast<BufferAttacher *>(cursorSurface->bufferAttacher())->image();
    QCursor cursor(QPixmap::fromImage(image), cursorHotspotX, cursorHotspotY);

    static bool cursorIsSet = false;
    if (cursorIsSet) {
        QGuiApplication::changeOverrideCursor(cursor);
    } else {
        QGuiApplication::setOverrideCursor(cursor);
        cursorIsSet = true;
    }
#endif
}

void CompositorPrivate::addWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    if (!clientWindowsList.contains(window)) {
        clientWindowsList.append(window);
        Q_EMIT q->windowsChanged();
    }
}

void CompositorPrivate::removeWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    if (clientWindowsList.removeOne(window))
        Q_EMIT q->windowsChanged();
}

void CompositorPrivate::mapWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    addWindow(window);

    Q_EMIT appManager->windowMapped(window);
    Q_EMIT q->windowMapped(QVariant::fromValue(window));
}

void CompositorPrivate::unmapWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    removeWindow(window);

    Q_EMIT appManager->windowUnmapped(window);
    Q_EMIT q->windowUnmapped(QVariant::fromValue(window));
}

void CompositorPrivate::destroyWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    removeWindow(window);

    Q_EMIT appManager->windowUnmapped(window);
    Q_EMIT q->windowDestroyed(window->id());
}

void CompositorPrivate::mapShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (!shellWindowsList.contains(window)) {
        shellWindowsList.append(window);
        Q_EMIT q->shellWindowMapped(QVariant::fromValue(window));
        Q_EMIT q->shellWindowsChanged();
    }
}

void CompositorPrivate::unmapShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (shellWindowsList.removeOne(window)) {
        Q_EMIT q->shellWindowUnmapped(QVariant::fromValue(window));
        Q_EMIT q->shellWindowsChanged();
    }
}

void CompositorPrivate::destroyShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (shellWindowsList.removeOne(window)) {
        Q_EMIT q->shellWindowDestroyed(window->id());
        Q_EMIT q->shellWindowsChanged();
    }
}

/*
 * Compositor
 */

Compositor::Compositor(const QString &socket)
    : QWaylandQuickCompositor(socket.isEmpty() ? 0 : qPrintable(socket), WindowManagerExtension | QtKeyExtension | TouchExtension | HardwareIntegrationExtension | SubSurfaceExtension)
    , d_ptr(new CompositorPrivate(this))
{
}

Compositor::~Compositor()
{
    // Cleanup
    while (!d_ptr->shellWindowsList.isEmpty())
        d_ptr->shellWindowsList.takeFirst()->deleteLater();
    while (!d_ptr->clientWindowsList.isEmpty())
        d_ptr->clientWindowsList.takeFirst()->deleteLater();
    delete d_ptr->appManager;
    delete d_ptr->screenManager;
    delete d_ptr;

    // Cleanup graphics resources
    qCDebug(GREENISLAND_COMPOSITOR) << "Cleanup graphics resources...";
    cleanupGraphicsResources();

    // Delete windows and outputs
    qCDebug(GREENISLAND_COMPOSITOR) << "Closing all remaining windows...";
    for (QWaylandOutput *output: outputs()) {
        if (output->window())
            output->window()->deleteLater();
        output->deleteLater();
    }
}

Compositor::State Compositor::state() const
{
    Q_D(const Compositor);
    return d->state;
}

void Compositor::setState(Compositor::State state)
{
    Q_D(Compositor);

    if (state == Compositor::Active && d->state == state) {
        Q_EMIT idleInhibitResetRequested();
        Q_EMIT idleTimerStartRequested();
        return;
    }

    if (d->state != state) {
        switch (state) {
        case Compositor::Active:
            switch (d->state) {
            case Compositor::Sleeping:
                d->dpms(true);
            default:
                Q_EMIT wake();
                Q_EMIT idleInhibitResetRequested();
                Q_EMIT idleTimerStartRequested();
            }
        case Compositor::Idle:
            Q_EMIT idle();
            Q_EMIT idleInhibitResetRequested();
            Q_EMIT idleTimerStopRequested();
            break;
        case Compositor::Offscreen:
            switch (d->state) {
            case Compositor::Sleeping:
                d->dpms(true);
            default:
                Q_EMIT idleInhibitResetRequested();
                Q_EMIT idleTimerStopRequested();
            }
        case Compositor::Sleeping:
            Q_EMIT idleInhibitResetRequested();
            Q_EMIT idleTimerStopRequested();
            d->dpms(false);
            break;
        }

        d->state = state;
        Q_EMIT stateChanged();
    }
}

int Compositor::idleInterval() const
{
    Q_D(const Compositor);
    return d->idleInterval;
}

void Compositor::setIdleInterval(int value)
{
    Q_D(Compositor);

    if (d->idleInterval != value) {
        d->idleInterval = value;
        Q_EMIT idleIntervalChanged();
    }
}

int Compositor::idleInhibit() const
{
    Q_D(const Compositor);
    return d->idleInhibit;
}

void Compositor::setIdleInhibit(int value)
{
    Q_D(Compositor);

    if (d->idleInhibit != value) {
        d->idleInhibit = value;
        Q_EMIT idleInhibitChanged();
    }
}

bool Compositor::isLocked() const
{
    Q_D(const Compositor);

    return d->locked;
}

void Compositor::setLocked(bool value)
{
    Q_D(Compositor);

    if (d->locked == value)
        return;

    d->locked = value;
    Q_EMIT lockedChanged();
}

CompositorSettings *Compositor::settings() const
{
    Q_D(const Compositor);
    return d->settings;
}

ScreenManager *Compositor::screenManager() const
{
    Q_D(const Compositor);
    return d->screenManager;
}

ApplicationManager *Compositor::applicationManager() const
{
    Q_D(const Compositor);
    return d->appManager;
}

void Compositor::run()
{
    Q_D(Compositor);

    if (d->running)
        return;

    // Add global interfaces
    d->recorderManager = new GreenIslandRecorderManager();
    addGlobalInterface(d->recorderManager);
    addGlobalInterface(new GreenIslandApps(d->appManager));
    addGlobalInterface(new GreenIslandWindows(d->appManager));
    PlasmaShell *plasmaShell = new PlasmaShell(this);
    addGlobalInterface(plasmaShell);
    addGlobalInterface(new PlasmaEffects(plasmaShell));
    addGlobalInterface(new WlShellGlobal());
    addGlobalInterface(new XdgShellGlobal());
    addGlobalInterface(new GtkShellGlobal());

    d->running = true;

#if HAVE_SYSTEMD
    qCDebug(GREENISLAND_COMPOSITOR) << "Compositor ready, notify systemd on" << qgetenv("NOTIFY_SOCKET");
    sd_notify(0, "READY=1");
#endif
}

QWaylandSurfaceView *Compositor::pickView(const QPointF &globalPosition) const
{
    Q_D(const Compositor);

    // TODO: Views should probably ordered by z-index in order to really
    // pick the first view with that global coordinates

    for (QWaylandOutput *output: m_compositor->outputs()) {
        if (output->geometry().contains(globalPosition.toPoint())) {
            for (QWaylandSurface *surface: output->surfaces()) {
                for (ClientWindow *window: d->clientWindowsList) {
                    if (window->surface() != surface)
                        continue;
                    if (window->geometry().contains(globalPosition))
                        return surface->views().at(0);
                }
            }
        }
    }

    return Q_NULLPTR;
}

QWaylandSurfaceItem *Compositor::firstViewOf(QWaylandSurface *surface)
{
    if (!surface) {
        qCWarning(GREENISLAND_COMPOSITOR) << "First view of null surface requested!";
        return Q_NULLPTR;
    }

    return static_cast<QWaylandSurfaceItem *>(surface->views().first());
}

void Compositor::surfaceCreated(QWaylandSurface *surface)
{
    Q_D(Compositor);

    // Connect surface signals
    connect(surface, &QWaylandSurface::mapped, [=] {
        Q_EMIT surfaceMapped(QVariant::fromValue(surface));
    });
    connect(surface, &QWaylandSurface::unmapped, [=] {
        Q_EMIT surfaceUnmapped(QVariant::fromValue(surface));
    });
    connect(surface, &QWaylandSurface::surfaceDestroyed, [=] {
        Q_EMIT surfaceDestroyed(QVariant::fromValue(surface));
    });
}

QWaylandSurfaceView *Compositor::createView(QWaylandSurface *surf)
{
    return new WindowView(qobject_cast<QWaylandQuickSurface *>(surf));
}

void Compositor::clearKeyboardFocus()
{
    Q_D(Compositor);

    d->lastKeyboardFocus = defaultInputDevice()->keyboardFocus();
    defaultInputDevice()->setKeyboardFocus(Q_NULLPTR);
}

void Compositor::restoreKeyboardFocus()
{
    Q_D(Compositor);

    if (d->lastKeyboardFocus) {
        defaultInputDevice()->setKeyboardFocus(d->lastKeyboardFocus);
        d->lastKeyboardFocus = Q_NULLPTR;
    }
}

void Compositor::abortSession()
{
    QGuiApplication::quit();
}

void Compositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
//#ifdef QT_COMPOSITOR_WAYLAND_GL
#if 0
    Q_D(Compositor);

    // Setup cursor
    d->cursorHotspotX = hotspotX;
    d->cursorHotspotY = hotspotY;

    if ((d->cursorSurface != surface) && surface) {
        // Buffer attacher
        d->cursorSurface = surface;
        d->cursorSurface->setBufferAttacher(new BufferAttacher());

        // Update cursor when mapped
        connect(surface, SIGNAL(configure(bool)), this, SLOT(_q_updateCursor(bool)));
    }
#else
    Q_UNUSED(surface);
    Q_UNUSED(hotspotX);
    Q_UNUSED(hotspotY);
#endif
}

}

#include "moc_compositor.cpp"
