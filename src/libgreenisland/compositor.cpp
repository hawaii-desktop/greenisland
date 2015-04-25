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
#include <QtCompositor/QWaylandClient>
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

#include "client/wlclientconnection.h"
#include "client/wlregistry.h"
#include "protocols/greenisland/greenislandapps.h"
#include "protocols/greenisland/greenislandwindows.h"
#include "protocols/greenisland/greenislandrecorder.h"
#include "protocols/gtk-shell/gtkshell.h"
#include "protocols/wl-shell/wlshell.h"
#include "protocols/xdg-shell/xdgshell.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

#include <wayland-client-protocol.h>

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
    connect(d->screenManager, &ScreenManager::configurationAcquired, this, [this, d] {
        // Emit a signal which is handy for plugins
        Q_EMIT screenConfigurationAcquired();

#if HAVE_SYSTEMD
        // Notify systemd when the screen configuration is ready
        qCDebug(GREENISLAND_COMPOSITOR) << "Compositor ready, notify systemd on" << qgetenv("NOTIFY_SOCKET");
        sd_notify(0, "READY=1");
#endif

        // Start idle timer
        connect(d->idleTimer, &QTimer::timeout, this, [this, d] {
            if (d->idleInhibit == 0)
                setState(Idle);
        });
        d->idleTimer->start();
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

QList<KeyBinding> Compositor::keyBindings() const
{
    Q_D(const Compositor);
    return d->keyBindings;
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
    addGlobalInterface(new XdgShellGlobal(this));
    addGlobalInterface(new GtkShellGlobal());

    // Add global interfaces from plugins
    Q_FOREACH (AbstractPlugin *plugin, d->plugins)
        plugin->addGlobalInterfaces();

    // Connect to the main compositor if we are nested into another compositor and
    // queue internal connection creation; we need to create an internal connection
    // in any case to be able to create the shm pool used by cursor themes
    if (d->nested) {
        // Queue nested connection initialization, internal connection
        // will be initialized once the connection to the main compositor
        // is established
        QMetaObject::invokeMethod(this, "_q_createNestedConnection",
                                  Qt::QueuedConnection);
    } else {
        // Queue internal connection initialization
        QMetaObject::invokeMethod(this, "_q_createInternalConnection",
                                  Qt::QueuedConnection);

        // Queue screen configuration acquisition
        QMetaObject::invokeMethod(d->screenManager, "acquireConfiguration",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, d->fakeScreenConfiguration));
    }
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

bool Compositor::registerKeyBinding(const QString &name, const QString &keys)
{
    Q_D(Compositor);

    // Do we already have this keybinding?
    Q_FOREACH (const KeyBinding &binding, d->keyBindings) {
        if (binding.name() == name)
            return false;
    }

    // Is the keybinding text valid?
    QKeySequence sequence(keys);
    if (!sequence.isEmpty()) {
        d->keyBindings.append(KeyBinding(name, sequence));
        return true;
    }

    return false;
}

bool Compositor::unregisterKeyBinding(const QString &name)
{
    Q_D(Compositor);

    Q_FOREACH (const KeyBinding &binding, d->keyBindings) {
        if (binding.name() == name) {
            d->keyBindings.removeOne(binding);
            return true;
        }
    }

    return false;
}


void Compositor::abortSession()
{
    QGuiApplication::quit();
}

void Compositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    // Sanity check
    if (!surface)
        return;

#if (defined QT_COMPOSITOR_WAYLAND_GL) && (QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 4, 2))
    Q_D(Compositor);

    // Do not change the cursor if the compositor wants to
    // exclusively change it (for example during a window move operation)
    if (d->grabCursor && surface->client()->client() != d->clientData.client)
        return;

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
