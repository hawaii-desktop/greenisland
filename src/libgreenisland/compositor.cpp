/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QTimer>
#include <QtGui/QGuiApplication>
#include <QtCompositor/QtCompositorVersion>
#include <QtCompositor/private/qwlcompositor_p.h>

#include "abstractplugin.h"
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
#include "output.h"
#include "windowview.h"
#include "shellwindow.h"

#include "protocols/greenisland/greenislandapps.h"
#include "protocols/greenisland/greenislandwindows.h"
#include "protocols/greenisland/greenislandrecorder.h"
#include "protocols/gtk-shell/gtkshell.h"
#include "protocols/wl-shell/wlshell.h"
#include "protocols/xdg-shell/xdgshell.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

namespace GreenIsland {

QString Compositor::s_fixedShell;

Compositor::Compositor(const QString &socket)
    : QWaylandQuickCompositor(socket.isEmpty() ? 0 : qPrintable(socket), WindowManagerExtension | QtKeyExtension | TouchExtension | HardwareIntegrationExtension | SubSurfaceExtension)
    , d_ptr(new CompositorPrivate(this))
{
    Q_D(Compositor);

    // Load plugins
    d->loadPlugins();

    // Actions to do when the screen configuration is acquired
    connect(d->screenManager, &ScreenManager::configurationAcquired, this, [this] {
        // Emit a signal which is handy for plugins
        Q_EMIT screenConfigurationAcquired();

#if HAVE_SYSTEMD
        // Notify systemd when the screen configuration is ready
        qCDebug(GREENISLAND_COMPOSITOR) << "Compositor ready, notify systemd on" << qgetenv("NOTIFY_SOCKET");
        sd_notify(0, "READY=1");
#endif
    });
}

Compositor::~Compositor()
{
    // Cleanup
    while (!d_ptr->shellWindowsList.isEmpty())
        d_ptr->shellWindowsList.takeFirst()->deleteLater();
    while (!d_ptr->clientWindowsList.isEmpty())
        d_ptr->clientWindowsList.takeFirst()->deleteLater();
    delete d_ptr;

    // Cleanup graphics resources
    qCDebug(GREENISLAND_COMPOSITOR) << "Cleanup graphics resources...";
    cleanupGraphicsResources();
}

void Compositor::setFakeScreenConfiguration(const QString &fileName)
{
    Q_D(Compositor);
    d->fakeScreenConfiguration = fileName;
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
        d->idleInhibit = 0;
        d->idleTimer->start();
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
                d->idleInhibit = 0;
                d->idleTimer->start();
            }
            break;
        case Compositor::Idle:
            d->idleInhibit = 0;
            d->idleTimer->stop();
            Q_EMIT idle();
            break;
        case Compositor::Offscreen:
            switch (d->state) {
            case Compositor::Sleeping:
                d->dpms(true);
            default:
                d->idleInhibit = 0;
                d->idleTimer->stop();
            }
        case Compositor::Sleeping:
            d->idleInhibit = 0;
            d->idleTimer->stop();
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
    return d->idleTimer->interval();
}

void Compositor::setIdleInterval(int value)
{
    Q_D(Compositor);

    if (d->idleTimer->interval() != value) {
        d->idleTimer->setInterval(value);
        d->idleTimer->start();
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

void Compositor::incrementIdleInhibit()
{
    setIdleInhibit(idleInhibit() + 1);
}

void Compositor::decrementIdleInhibit()
{
    setIdleInhibit(idleInhibit() - 1);
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

void Compositor::run()
{
    Q_D(Compositor);

    // Can call run only once
    if (d->running)
        return;
    d->running = true;

    // Add global interfaces
    d->recorderManager = new GreenIslandRecorderManager();
    addGlobalInterface(d->recorderManager);
    addGlobalInterface(new GreenIslandApps());
    addGlobalInterface(new GreenIslandWindows());
    addGlobalInterface(new WlShellGlobal());
    addGlobalInterface(new XdgShellGlobal());
    addGlobalInterface(new GtkShellGlobal());

    // Add global interfaces from plugins
    Q_FOREACH (AbstractPlugin *plugin, d->plugins)
        plugin->addGlobalInterfaces();

    // Create outputs
    d->screenManager->acquireConfiguration(d->fakeScreenConfiguration);

    // Start idle timer
    connect(d->idleTimer, &QTimer::timeout, this, [this, d] {
        if (d->idleInhibit == 0)
            setState(Idle);
    });
    d->idleTimer->start();
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

    qCDebug(GREENISLAND_COMPOSITOR) << "Surface created" << surface;

    Q_EMIT newSurfaceCreated(surface);

    // Connect surface signals
    connect(surface, &QWaylandSurface::mapped, this, [this, surface] {
        Q_EMIT surfaceMapped(surface);
    });
    connect(surface, &QWaylandSurface::unmapped, this, [this, surface] {
        Q_EMIT surfaceUnmapped(surface);
    });
    connect(surface, &QWaylandSurface::surfaceDestroyed, this, [this, surface] {
        Q_EMIT surfaceDestroyed(surface);
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
#if (defined QT_COMPOSITOR_WAYLAND_GL) && (QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 4, 2))
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
