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

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtCore/QVariantMap>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/QWaylandOutput>
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "applicationmanager.h"
#include "applicationmanager_p.h"
#ifdef QT_COMPOSITOR_WAYLAND_GL
#  include "bufferattacher.h"
#endif
#include "cmakedirs.h"
#include "clientwindow.h"
#include "compositor.h"
#include "config.h"
#include "quicksurface.h"
#include "windowview.h"
#include "screenmanager.h"
#include "shellwindowview.h"

#include "protocols/plasma/plasmaeffects.h"
#include "protocols/plasma/plasmashell.h"
#include "protocols/wl-shell/wlshell.h"
#include "protocols/xdg-shell/xdgshell.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

typedef QList<QWaylandSurface *> QWaylandSurfaceList;

namespace GreenIsland {

QString Compositor::s_fixedPlugin;

/*
 * CompositorPrivate
 */

class CompositorPrivate
{
public:
    CompositorPrivate(Compositor *self);

    void dpms(bool on);

    void _q_updateCursor(bool hasBuffer);

    bool running;

    Compositor::State state;
    int idleInterval;
    int idleInhibit;

    // Cursor
    QWaylandSurface *cursorSurface;
    int cursorHotspotX;
    int cursorHotspotY;

    ScreenManager *screenManager;

    // Application management
    ApplicationManager *appManager;
    QHash<QString, QWaylandSurfaceList> appSurfaces;
    QList<ClientWindow *> clientWindows;

protected:
    Q_DECLARE_PUBLIC(Compositor)
    Compositor *const q_ptr;
};

CompositorPrivate::CompositorPrivate(Compositor *self)
    : running(false)
    , state(Compositor::Active)
    , idleInterval(5 * 60000)
    , idleInhibit(0)
    , cursorSurface(Q_NULLPTR)
    , cursorHotspotX(0)
    , cursorHotspotY(0)
    , q_ptr(self)
{
    screenManager = new ScreenManager(self);
    appManager = new ApplicationManager(self);
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

/*
 * Compositor
 */

Compositor::Compositor(const QString &socket)
    : QWaylandQuickCompositor(socket.isEmpty() ? 0 : qPrintable(socket), WindowManagerExtension | OutputExtension | QtKeyExtension | TouchExtension | HardwareIntegrationExtension | SubSurfaceExtension)
    , d_ptr(new CompositorPrivate(this))
{
    qRegisterMetaType<QuickSurface *>("QuickSurface*");
    qRegisterMetaType<Output *>("Output*");
    qRegisterMetaType<WindowView *>("WindowView*");
    qRegisterMetaType<ShellWindowView *>("ShellWindowView*");
}

Compositor::~Compositor()
{
    // Cleanup
    delete d_ptr->appManager;
    delete d_ptr->screenManager;
    while (!d_ptr->clientWindows.isEmpty())
        d_ptr->clientWindows.takeFirst()->deleteLater();
    delete d_ptr;

    // Delete windows and outputs
    qDebug() << "Closing all remaining windows...";
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
    addGlobalInterface(new PlasmaEffects());
    addGlobalInterface(new PlasmaShell(this));
    addGlobalInterface(new WlShell());
    addGlobalInterface(new XdgShell());

    d->running = true;

#if HAVE_SYSTEMD
    qDebug() << "Compositor ready, notify systemd on" << qgetenv("NOTIFY_SOCKET");
    sd_notify(0, "READY=1");
#endif
}

QWaylandSurface *Compositor::createSurface(QWaylandClient *client, quint32 id, int version)
{
    return new QuickSurface(client, id, version, this);
}

QWaylandSurfaceView *Compositor::pickView(const QPointF &globalPosition) const
{
    // TODO: Views should probably ordered by z-index in order to really
    // pick the first view with that global coordinates

    for (QWaylandOutput *output: m_compositor->outputs()) {
        if (output->geometry().contains(globalPosition.toPoint())) {
            for (QWaylandSurface *surface: output->surfaces()) {
                QuickSurface *quickSurface = qobject_cast<QuickSurface *>(surface);
                if (!quickSurface)
                    continue;

                if (quickSurface->globalGeometry().contains(globalPosition))
                    return quickSurface->views().at(0);
            }
        }
    }

    return Q_NULLPTR;
}

QWaylandSurfaceItem *Compositor::firstViewOf(QuickSurface *surface)
{
    if (!surface) {
        qWarning() << "First view of null surface requested!";
        return nullptr;
    }

    return static_cast<WindowView *>(surface->views().first());
}

QWaylandSurfaceItem *Compositor::viewForOutput(QuickSurface *surface, Output *output)
{
    if (!surface) {
        qWarning() << "View for a null surface requested!";
        return Q_NULLPTR;
    }

    if (!output) {
        qWarning() << "View for a null output requested!";
        return Q_NULLPTR;
    }

    // Search a view for this output
    for (QWaylandSurfaceView *surfaceView: surface->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;

        if (view->output() == output)
            return view;
    }

    // None was found: create one
    return new WindowView(surface, output);
}

void Compositor::surfaceCreated(QWaylandSurface *surface)
{
    if (!surface)
        return;

    Q_D(Compositor);

    // Connect surface signals
    connect(surface, &QWaylandSurface::mapped, [=]() {
        // Let the QML compositor handle mapped surfaces
        Q_EMIT surfaceMapped(QVariant::fromValue(surface));

        // Only toplevel windows qualify as important
        if (surface->windowType() != QWaylandSurface::Toplevel)
            return;

        // Create application window instance
        ClientWindow *appWindow = new ClientWindow(surface);
        d->clientWindows.append(appWindow);

        // Register application only if it's the first time this surface is mapped
        const QString appId = surface->className();
        if (!d->appSurfaces[appId].contains(surface)) {
            d->appManager->d_func()->registerApplication(appId);
            d->appSurfaces[appId].append(surface);
        }
    });
    connect(surface, &QWaylandSurface::unmapped, [=]() {
        // Let the QML compositor handle unmapped surfaces
        Q_EMIT surfaceUnmapped(QVariant::fromValue(surface));

        // Unregister application if the last surface is gone
        const QString appId = surface->className();
        d->appSurfaces[appId].removeOne(surface);
        if (d->appSurfaces[appId].isEmpty())
            d->appManager->d_func()->unregisterApplication(appId);

        // Delete application window
        QList<ClientWindow *>::iterator i = d->clientWindows.begin();
        while (i != d->clientWindows.end()) {
            ClientWindow *appWindow = (*i);
            if (appWindow->surface() == surface) {
                i = d->clientWindows.erase(i);
                appWindow->deleteLater();
                break;
            } else {
                ++i;
            }
        }
    });
    connect(surface, &QWaylandSurface::surfaceDestroyed, [=]() {
        // Let the QML compositor handle surface destruction
        Q_EMIT surfaceDestroyed(QVariant::fromValue(surface));
    });
}

QPointF Compositor::calculateInitialPosition(QWaylandSurface *surface)
{
    // As a heuristic place the new window on the same output as the
    // pointer. Falling back to the output containing 0,0.
    // TODO: Do something clever for touch too
    QPointF pos = defaultInputDevice()->handle()->pointerDevice()->currentPosition();

    // Find the target screen (the one where the coordinates are in)
    QRect geometry;
    bool targetScreenFound = false;
    for (int i = 0; i < outputs().size(); i++) {
        geometry = outputs().at(i)->availableGeometry();
        if (geometry.contains(pos.toPoint())) {
            targetScreenFound = true;
            break;
        }
    }

    // Just move the surface to a random position if we can't find a target output
    if (!targetScreenFound) {
        pos.setX(10 + qrand() % 400);
        pos.setY(10 + qrand() % 400);
        return pos;
    }

    // Valid range within output where the surface will still be onscreen.
    // If this is negative it means that the surface is bigger than
    // output in this case we fallback to 0,0 in available geometry space.
    int rangeX = geometry.size().width() - surface->size().width();
    int rangeY = geometry.size().height() - surface->size().height();

    int dx = 0, dy = 0;
    if (rangeX > 0)
        dx = qrand() % rangeX;
    if (rangeY > 0)
        dy = qrand() % rangeY;

    // Set surface position
    pos.setX(geometry.x() + dx);
    pos.setY(geometry.y() + dy);

    return pos;
}

void Compositor::abortSession()
{
    QGuiApplication::quit();
}

QQmlListProperty<ClientWindow> Compositor::windows()
{
    Q_D(Compositor);
    return QQmlListProperty<ClientWindow>(this, d->clientWindows);
}

int Compositor::windowCount() const
{
    Q_D(const Compositor);
    return d->clientWindows.count();
}

ClientWindow *Compositor::window(int index) const
{
    Q_D(const Compositor);
    return d->clientWindows.at(index);
}

void Compositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
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
