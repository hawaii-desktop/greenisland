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

#include "bufferattacher.h"
#include "cmakedirs.h"
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

    void _q_outputRemoved(QWaylandOutput *_output);

    bool running;

    Compositor::State state;
    int idleInterval;
    int idleInhibit;

    // Cursor
    QWaylandSurface *cursorSurface;
    int cursorHotspotX;
    int cursorHotspotY;

    ScreenManager *screenManager;

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
}

void CompositorPrivate::dpms(bool on)
{
    // TODO
}

void CompositorPrivate::_q_updateCursor(bool hasBuffer)
{
    if (!hasBuffer || !cursorSurface || !cursorSurface->bufferAttacher())
        return;

    QImage image = static_cast<BufferAttacher *>(cursorSurface->bufferAttacher())->image();
    QCursor cursor(QPixmap::fromImage(image), cursorHotspotX, cursorHotspotY);

    static bool cursorIsSet = false;
    if (cursorIsSet) {
        QGuiApplication::changeOverrideCursor(cursor);
    } else {
        QGuiApplication::setOverrideCursor(cursor);
        cursorIsSet = true;
    }
}

void CompositorPrivate::_q_outputRemoved(QWaylandOutput *_output)
{
    Q_Q(Compositor);

    Output *output = qobject_cast<Output *>(_output);
    if (!output)
        return;

    // Remove all views created for this output
    for (QWaylandSurface *surface: q->surfaces()) {
        for (QWaylandSurfaceView *surfaceView: surface->views()) {
            WindowView *view = static_cast<WindowView *>(surfaceView);
            if (!view)
                continue;

            if (view->output() == output)
                view->deleteLater();
        }
    }
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

    connect(this, SIGNAL(outputRemoved(QWaylandOutput*)),
            this, SLOT(_q_outputRemoved(QWaylandOutput*)));
}

Compositor::~Compositor()
{
#if 0
    qDeleteAll(m_clientWindows);
    qDeleteAll(m_workspaces);
#endif
    delete d_ptr;
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

    // TODO: Load workspaces number from config
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceSelected(0);

    d->running = true;

#if HAVE_SYSTEMD
    qDebug() << "Compositor ready, notify systemd on" << qgetenv("NOTIFY_SOCKET");
    sd_notify(0, "READY=1");
#endif
}

QWaylandQuickSurface *Compositor::createSurface(wl_client *client, quint32 id)
{
    return new QuickSurface(client, id, this);
}

QWaylandSurfaceView *Compositor::pickView(const QPointF &globalPosition) const
{
    // TODO: Views should probably ordered by z-index in order to really
    // pick the first view with that global coordinates

    for (QtWayland::Surface *curSurface: m_compositor->surfaces()) {
        QuickSurface *surface = qobject_cast<QuickSurface *>(curSurface->waylandSurface());
        if (!surface)
            continue;

        if (surface->globalGeometry().contains(globalPosition))
            return surface->views().at(0);
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

    connect(surface, &QWaylandSurface::mapped, [=]() {
        Q_EMIT surfaceMapped(QVariant::fromValue(surface));
    });
    connect(surface, &QWaylandSurface::unmapped, [=]() {
        Q_EMIT surfaceUnmapped(QVariant::fromValue(surface));
    });
    connect(surface, &QWaylandSurface::surfaceDestroyed, [=]() {
        Q_EMIT surfaceDestroyed(QVariant::fromValue(surface));
    });

#if 0
    // Create application window instance
    ClientWindow *appWindow = new ClientWindow(waylandDisplay());
    appWindow->setSurface(surface);
    m_clientWindows.append(appWindow);

    // Delete application window on surface destruction
    connect(surface, &QWaylandSurface::destroyed, [=](QObject *object = 0) {
        for (ClientWindow *appWindow: m_clientWindows) {
            if (appWindow->surface() == surface) {
                if (m_clientWindows.removeOne(appWindow))
                    delete appWindow;
                break;
            }
        }
    });
#endif
}

QPointF Compositor::calculateInitialPosition(QWaylandSurface *surface)
{
    Q_D(Compositor);

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

void Compositor::lockSession()
{
}

void Compositor::unlockSession()
{
}

void Compositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
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
}

}

#include "moc_compositor.cpp"
