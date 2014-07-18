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
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickItem>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>

#include "bufferattacher.h"
#include "cmakedirs.h"
#include "compositor.h"
#include "screenmodel.h"

/*
 * CompositorPrivate
 */

class CompositorPrivate
{
public:
    CompositorPrivate(Compositor *self);

    void dpms(bool on);

    void _q_sendCallbacks();

    void _q_updateCursor(bool hasBuffer);

    void _q_surfaceDestroyed(QObject *object);
    void _q_surfaceMapped();
    void _q_surfaceUnmapped();

    Compositor::State state;
    int idleInterval;
    int idleInhibit;

    // Cursor
    QWaylandSurface *cursorSurface;
    int cursorHotspotX;
    int cursorHotspotY;

protected:
    Q_DECLARE_PUBLIC(Compositor)
    Compositor *const q_ptr;
};

CompositorPrivate::CompositorPrivate(Compositor *self)
    : state(Compositor::Active)
    , idleInterval(5 * 60000)
    , idleInhibit(0)
    , cursorSurface(nullptr)
    , cursorHotspotX(0)
    , cursorHotspotY(0)
    , q_ptr(self)
{
}

void CompositorPrivate::dpms(bool on)
{
    // TODO
}

void CompositorPrivate::_q_sendCallbacks()
{
    Q_Q(Compositor);
    q->sendFrameCallbacks(q->surfaces());
}

void CompositorPrivate::_q_updateCursor(bool hasBuffer)
{
    Q_UNUSED(hasBuffer);

    if (!cursorSurface)
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

void CompositorPrivate::_q_surfaceDestroyed(QObject *object)
{
    Q_Q(Compositor);

    QWaylandQuickSurface *surface = static_cast<QWaylandQuickSurface *>(object);
    Q_EMIT q->surfaceDestroyed(QVariant::fromValue(surface));
}

void CompositorPrivate::_q_surfaceMapped()
{
    Q_Q(Compositor);

    QWaylandQuickSurface *surface = qobject_cast<QWaylandQuickSurface *>(q->sender());
    Q_EMIT q->surfaceMapped(QVariant::fromValue(surface));
}

void CompositorPrivate::_q_surfaceUnmapped()
{
    Q_Q(Compositor);

    QWaylandQuickSurface *surface = qobject_cast<QWaylandQuickSurface *>(q->sender());
    Q_EMIT q->surfaceUnmapped(QVariant::fromValue(surface));
}

/*
 * Compositor
 */

Compositor::Compositor(const QString &socket)
    : QWaylandQuickCompositor(this, socket.isEmpty() ? 0 : qPrintable(socket), DefaultExtensions | SubSurfaceExtension)
    , d_ptr(new CompositorPrivate(this))
{
    qmlRegisterType<Compositor>("GreenIsland.Core", 1, 0, "Compositor");
    qmlRegisterType<ScreenModel>("GreenIsland.Core", 1, 0, "ScreenModel");
    rootContext()->setContextProperty("compositor", this);

    setSource(QUrl("qrc:/qml/Compositor.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::black);
    winId();
    addDefaultShell();

    connect(this, SIGNAL(afterRendering()),
            this, SLOT(_q_sendCallbacks()));

    // TODO: Load workspaces number from config
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceAdded();
    Q_EMIT workspaceSelected(0);

    // FIXME: Fade in immediately, when there will be a protocol for the shell
    // we will fade in as soon as it tells us it's ready
    Q_EMIT fadeIn();
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

QWaylandSurfaceItem *Compositor::firstViewOf(QWaylandSurface *surface)
{
    if (!surface) {
        qWarning() << "First view of null surface requested!";
        return nullptr;
    }

    return static_cast<QWaylandSurfaceItem *>(surface->views().first());
}

void Compositor::surfaceCreated(QWaylandSurface *surface)
{
    if (!surface)
        return;

    connect(surface, SIGNAL(destroyed(QObject*)), this, SLOT(_q_surfaceDestroyed(QObject*)));
    connect(surface, SIGNAL(mapped()), this, SLOT(_q_surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this, SLOT(_q_surfaceUnmapped()));

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
    // As a heuristic place the new window on the same output as the
    // pointer. Falling back to the output containing 0,0.
    // TODO: Do something clever for touch too
    QPointF pos = defaultInputDevice()->handle()->pointerDevice()->currentPosition();

    // Find the target output (the one where the coordinates are in)
    // FIXME: QtWayland::Compositor should give us a list of outputs and then we
    // can scroll the list and find the output that contains the coordinates;
    // targetOutput then would be a pointer to an Output
    QtWayland::Compositor *compositor = static_cast<QWaylandCompositor *>(this)->handle();
    bool targetOutput = compositor->outputGeometry().contains(pos.toPoint());

    // Just move the surface to a random position if we can't find a target output
    if (!targetOutput) {
        pos.setX(10 + qrand() % 400);
        pos.setY(10 + qrand() % 400);
        return pos;
    }

    // Valid range within output where the surface will still be onscreen.
    // If this is negative it means that the surface is bigger than
    // output in this case we fallback to 0,0 in available geometry space.
    // FIXME: When we have an Output we need a way to check the available
    // geometry (which is the portion of screen that is not used by panels)
    int rangeX = compositor->outputGeometry().width() - surface->size().width();
    int rangeY = compositor->outputGeometry().height() - surface->size().height();

    int dx = 0, dy = 0;
    if (rangeX > 0)
        dx = qrand() % rangeX;
    if (rangeY > 0)
        dy = qrand() % rangeY;

    // Set surface position
    pos.setX(compositor->outputGeometry().x() + dx);
    pos.setY(compositor->outputGeometry().y() + dy);

    return pos;
}

void Compositor::lockSession()
{
}

void Compositor::unlockSession()
{
}

#if 0
void Compositor::keyPressEvent(QKeyEvent *event)
{
    // Decode key event
    uint32_t modifiers = 0;
    uint32_t key = 0;

    if (event->modifiers() & Qt::ControlModifier)
        modifiers |= MODIFIER_CTRL;
    if (event->modifiers() & Qt::AltModifier)
        modifiers |= MODIFIER_ALT;
    if (event->modifiers() & Qt::MetaModifier)
        modifiers |= MODIFIER_SUPER;
    if (event->modifiers() & Qt::ShiftModifier)
        modifiers |= MODIFIER_SHIFT;

    int k = 0;
    while (keyTable[k]) {
        if (event->key() == (int)keyTable[k]) {
            key = keyTable[k + 1];
            break;
        }
        k += 2;
    }

    // Look for a matching key binding
    for (KeyBinding *keyBinding: m_shell->keyBindings()) {
        if (keyBinding->modifiers() == modifiers && keyBinding->key() == key) {
            keyBinding->send_triggered();
            break;
        }
    }

    // Call overridden method
    QWaylandCompositor::keyPressEvent(event);
}
#endif

void Compositor::keyPressEvent(QKeyEvent *event)
{
    Q_D(Compositor);
    d->idleInhibit++;

    QQuickView::keyPressEvent(event);
}

void Compositor::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(Compositor);
    d->idleInhibit--;

    QQuickView::keyReleaseEvent(event);
}

void Compositor::mousePressEvent(QMouseEvent *event)
{
    Q_D(Compositor);
    d->idleInhibit++;

    QQuickView::mousePressEvent(event);
}

void Compositor::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(Compositor);
    d->idleInhibit--;

    QQuickView::mouseReleaseEvent(event);
}

void Compositor::mouseMoveEvent(QMouseEvent *event)
{
    setState(Active);

    QQuickView::mouseMoveEvent(event);
}

void Compositor::wheelEvent(QWheelEvent *event)
{
    setState(Active);

    QQuickView::wheelEvent(event);
}

void Compositor::resizeEvent(QResizeEvent *event)
{
    QQuickView::resizeEvent(event);
    QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

void Compositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    Q_D(Compositor);

    // FIXME: Temporary disable this because it crashes on GLX
    return;

    if ((d->cursorSurface != surface) && surface) {
        // Set surface role
        surface->setWindowProperty(QStringLiteral("role"), CursorRole);

        // Update cursor
        connect(surface, SIGNAL(configure(bool)), this, SLOT(_q_updateCursor(bool)));
    }

    // Setup cursor
    d->cursorSurface = surface;
    d->cursorHotspotX = hotspotX;
    d->cursorHotspotY = hotspotY;
    if (!d->cursorSurface->bufferAttacher())
        d->cursorSurface->setBufferAttacher(new BufferAttacher());
}

#include "moc_compositor.cpp"
