/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCompositor/QWaylandSurfaceItem>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>

#include <qpa/qplatformnativeinterface.h>

#include "greenislandcompositor.h"
#include "greenislandcompositor_p.h"
#include "gldebug.h"
#include "logging.h"

Q_LOGGING_CATEGORY(GREENISLAND_COMPOSITOR, "greenisland.compositor")
Q_LOGGING_CATEGORY(GREENISLAND_CLIENT, "greenisland.client")

using namespace GreenIsland;

/*
 * CompositorPrivate
 */

CompositorPrivate::CompositorPrivate(Compositor *parent)
    : q_ptr(parent)
    , state(Compositor::CompositorActive)
    , shellProcess(nullptr)
    , idleInterval(5*60000)
    , idleTimer(new QTimer(parent))
    , idleInhibit(0)
{
    // Set default idle interval
    idleTimer->setInterval(idleInterval);
}

CompositorPrivate::~CompositorPrivate()
{
    closeShell();
}

void CompositorPrivate::closeShell()
{
    if (!shellProcess)
        return;

    shellProcess->close();
    delete shellProcess;
    shellProcess = nullptr;

    Q_Q(Compositor);
    Q_EMIT q->shellClientRunningChanged(false);
}

void CompositorPrivate::dpms(bool on)
{
    // TODO
}

void CompositorPrivate::_q_shellStarted()
{
    Q_Q(Compositor);
    Q_EMIT q->shellClientRunningChanged(true);
}

void CompositorPrivate::_q_shellFailed(QProcess::ProcessError error)
{
    Q_Q(Compositor);

    switch (error) {
    case QProcess::FailedToStart:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "The shell process failed to start.\n"
                << "Either the invoked program is missing, or you may have insufficient permissions to run it.";
        break;
    case QProcess::Crashed:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "The shell process crashed some time after starting successfully.";
        break;
    case QProcess::Timedout:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "The shell process timedout.\n";
        break;
    case QProcess::WriteError:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "An error occurred when attempting to write to the shell process.";
        break;
    case QProcess::ReadError:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "An error occurred when attempting to read from the shell process.";
        break;
    case QProcess::UnknownError:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "Unknown error starting the shell process!";
        break;
    }

    // Don't need it anymore because it failed
    shellProcess->close();
    delete shellProcess;
    shellProcess = nullptr;
    Q_EMIT q->shellClientRunningChanged(false);
}

void CompositorPrivate::_q_shellReadyReadStandardOutput()
{
    if (shellProcess)
        qCDebug(GREENISLAND_CLIENT) << qPrintable(shellProcess->readAllStandardOutput().trimmed());
}

void CompositorPrivate::_q_shellReadyReadStandardError()
{
    if (shellProcess)
        qCDebug(GREENISLAND_CLIENT) << qPrintable(shellProcess->readAllStandardError().trimmed());
}

void CompositorPrivate::_q_shellAboutToClose()
{
    qCDebug(GREENISLAND_COMPOSITOR) << "Shell is about to close...";
}

/*
 * Compositor
 */

Compositor::Compositor(const char *socketName, ExtensionFlags extensions)
    : QWaylandCompositor(this, socketName, extensions)
    , d_ptr(new CompositorPrivate(this))
{
    // Default window title
    setTitle(QLatin1String("GreenIsland"));

    // Allow QML to access this compositor
    rootContext()->setContextProperty("compositor", this);

    // Default settings
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::black);

    // Create platform window
    winId();
}

Compositor::~Compositor()
{
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

    if (state == Compositor::CompositorActive && d->state == state) {
        d->idleInhibit = 0;
        d->idleTimer->start();
        return;
    }

    if (d->state != state) {
        switch (state) {
        case Compositor::CompositorActive:
            switch (d->state) {
            case Compositor::CompositorSleeping:
                d->dpms(true);
            default:
                Q_EMIT wake();
                d->idleInhibit = 0;
                d->idleTimer->start();
            }
        case Compositor::CompositorIdle:
            Q_EMIT idle();
            d->idleInhibit = 0;
            d->idleTimer->stop();
            break;
        case Compositor::CompositorOffscreen:
            switch (d->state) {
            case Compositor::CompositorSleeping:
                d->dpms(true);
            default:
                d->idleInhibit = 0;
                d->idleTimer->stop();
            }
        case Compositor::CompositorSleeping:
            d->idleInhibit = 0;
            d->idleTimer->stop();
            d->dpms(false);
            break;
        }

        d->state = state;
        Q_EMIT stateChanged(state);
    }
}

QString Compositor::shellFileName() const
{
    Q_D(const Compositor);
    return d->shellFileName;
}

void Compositor::setShellFileName(const QString &fileName)
{
    Q_D(Compositor);

    if (d->shellFileName != fileName) {
        d->shellFileName = fileName;
        Q_EMIT shellFileNameChanged(fileName);
    }
}

QList<QWaylandSurface *> Compositor::surfaces() const
{
    Q_D(const Compositor);
    return d->surfaces;
}

void Compositor::showGraphicsInfo()
{
    printGraphicsInformation(window());
}

void Compositor::runShell(const QStringList &arguments)
{
    Q_D(Compositor);

    // Sanity check
    if (d->shellFileName.isEmpty() || d->shellProcess)
        return;

    // Run the shell client process
    d->shellProcess = new QProcess(this);
    connect(d->shellProcess, SIGNAL(started()),
            this, SLOT(_q_shellStarted()));
    connect(d->shellProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(_q_shellFailed(QProcess::ProcessError)));
    connect(d->shellProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(_q_shellReadyReadStandardOutput()));
    connect(d->shellProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(_q_shellReadyReadStandardError()));
    connect(d->shellProcess, SIGNAL(aboutToClose()),
            this, SLOT(_q_shellAboutToClose()));
    d->shellProcess->start(d->shellFileName, arguments, QIODevice::ReadOnly);
}

void Compositor::closeShell()
{
    Q_D(Compositor);
    d->closeShell();
}

bool Compositor::isShellClientRunning() const
{
    Q_D(const Compositor);
    return (d->shellProcess != nullptr);
}

quint32 Compositor::idleInterval() const
{
    Q_D(const Compositor);
    return d->idleInterval;
}

void Compositor::setIdleInterval(quint32 msecs)
{
    Q_D(Compositor);
    d->idleInterval = msecs;
    d->idleTimer->setInterval(d->idleInterval);
    if (d->idleTimer->isActive())
        d->idleTimer->start();
}

void Compositor::startIdleTimer()
{
    Q_D(Compositor);

    // Change state to idle
    connect(d->idleTimer, &QTimer::timeout, [=]() {
        // Just return if idle is inhibited by some activity
        if (d->idleInhibit > 0)
            return;

        // Change compositor state
        setState(Compositor::CompositorIdle);
    });
    d->idleTimer->start();
}

void Compositor::stopIdleTimer()
{
    Q_D(Compositor);
    d->idleTimer->stop();
}

void Compositor::surfaceCreated(QWaylandSurface *surface)
{
    Q_D(Compositor);

    d->surfaces.append(surface);

    connect(surface, &QWaylandSurface::mapped, [=]() {
        Q_EMIT surfaceMapped(surface);
    });
    connect(surface, &QWaylandSurface::unmapped, [=]() {
        Q_EMIT surfaceUnmapped(surface);
    });
    connect(surface, &QObject::destroyed, [=](QObject *object) {
        QWaylandSurface *surface = static_cast<QWaylandSurface *>(object);
        d->surfaces.removeOne(surface);
        Q_EMIT surfaceDestroyed(surface);
    });
}

void Compositor::surfaceAboutToBeDestroyed(QWaylandSurface *surface)
{
    // TODO:
}

void Compositor::damageAll()
{
#ifdef QT_COMPOSITOR_QUICK
    Q_D(Compositor);

    for (QWaylandSurface *surface: d->surfaces) {
        if (surface->surfaceItem())
            surface->surfaceItem()->setDamagedFlag(true);
    }
#else
    qCWarning() << "Can't damage surfaces because QtQuick support is not built in";
#endif
}

void Compositor::keyPressEvent(QKeyEvent *event)
{
    Q_D(Compositor);

    // Inhibit idle
    d->idleInhibit++;

    // Continue processing this event
    QQuickView::keyPressEvent(event);
}

void Compositor::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(Compositor);

    // Release idle
    d->idleInhibit--;

    // Continue processing this event
    QQuickView::keyReleaseEvent(event);
}

void Compositor::mousePressEvent(QMouseEvent *event)
{
    Q_D(Compositor);

    // Inhibit idle
    d->idleInhibit++;

    // Continue processing this event
    QQuickView::mousePressEvent(event);
}

void Compositor::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(Compositor);

    // Release idle
    d->idleInhibit--;

    // Continue processing this event
    QQuickView::mouseReleaseEvent(event);
}

void Compositor::mouseMoveEvent(QMouseEvent *event)
{
    // Wake up the compositor
    setState(Compositor::CompositorActive);

    // Continue processing this event
    QQuickView::mouseMoveEvent(event);
}

void Compositor::wheelEvent(QWheelEvent *event)
{
    // Wake up the compositor
    setState(Compositor::CompositorActive);

    // Continue processing this event
    QQuickView::wheelEvent(event);
}

void Compositor::touchEvent(QTouchEvent *event)
{
    Q_D(Compositor);

    if (event->type() == QEvent::TouchEnd) {
        // Release idle
        d->idleInhibit--;
    } else {
        // Inhibit idle
        d->idleInhibit++;
    }

    // Continue processing this event
    QQuickView::touchEvent(event);
}

void Compositor::resizeEvent(QResizeEvent *event)
{
    // Scale compositor output to window's size
    QQuickView::resizeEvent(event);
    QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

#include "moc_greenislandcompositor.cpp"
