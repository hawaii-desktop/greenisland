/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QTimer>
#include <QtGui/QGuiApplication>
#include "clientconnection.h"
#include "wayland_wrapper/qwlcompositor_p.h"

#include <typeinfo>

#include "abstractplugin.h"
#include "applicationmanager_p.h"
#include "cmakedirs.h"
#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "config.h"
#include "cursorbufferattacher_p.h"
#include "logging.h"
#include "output.h"
#include "windowview.h"
#include "shellwindow.h"
#include "surface.h"
#include "surfacebuffer.h"
#include "surfaceview.h"

#include "client/wlclientconnection.h"
#include "client/wlcursortheme.h"
#include "client/wlregistry.h"
#include "protocols/greenisland/greenislandapps.h"
#include "protocols/greenisland/greenislandwindows.h"
#include "protocols/greenisland/greenislandrecorder.h"
#include "protocols/greenisland/greenislandscreenshooter.h"
#include "protocols/gtk-shell/gtkshell.h"
#include "protocols/wl-shell/wlshell.h"
#include "protocols/wayland/wlsubcompositor.h"
#include "protocols/wayland/wlsubsurface.h"
#include "protocols/xdg-shell/xdgshell.h"

#include <wayland-client-protocol.h>

namespace GreenIsland {

class CompositorSingleton : public Compositor {};
Q_GLOBAL_STATIC(CompositorSingleton, s_compositor)

Compositor::Compositor(QObject *parent)
    : QObject(parent)
    , AbstractQuickCompositor(0, WindowManagerExtension | TouchExtension | HardwareIntegrationExtension)
    , d_ptr(new CompositorPrivate(this))
{
    Q_D(Compositor);

    // Register metatypes
    qRegisterMetaType<ApplicationManager *>("ApplicationManager*");
    qRegisterMetaType<ClientConnection*>("ClientConnection*");
    qRegisterMetaType<ClientWindow *>("ClientWindow*");
    qRegisterMetaType<CompositorSettings *>("CompositorSettings*");
    qRegisterMetaType<Output *>("Output*");
    qRegisterMetaType<Output::ModeFlags>("Output::Mode");
    qRegisterMetaType<Output::Subpixel>("Output::Subpixel");
    qRegisterMetaType<Output::Transform>("Output::Transform");
    qRegisterMetaType<ShellWindow *>("ShellWindow*");
    qRegisterMetaType<Surface*>("Surface*");
    qRegisterMetaType<SurfaceBuffer*>("SurfaceBuffer*");
    qRegisterMetaType<SurfaceView*>("SurfaceView*");
    qRegisterMetaType<SurfaceItem*>("SurfaceItem*");
    qRegisterMetaType<WindowView *>("WindowView*");

    // Load plugins
    d->loadPlugins();

    // Actions to do when the screen configuration is acquired
    connect(d->screenManager, &ScreenManager::configurationAcquired, this, [this, d] {
        // Emit a signal which is handy for plugins
        Q_EMIT screenConfigurationAcquired();

        // Start idle timer
        d->idleTimer->setInterval(d->idleInterval);
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
        delete d_ptr->shellWindowsList.takeFirst();
    while (!d_ptr->clientWindowsList.isEmpty())
        delete d_ptr->clientWindowsList.takeFirst();
    delete d_ptr;

    // Cleanup graphics resources
    qCDebug(GREENISLAND_COMPOSITOR) << "Cleanup graphics resources...";
    cleanupGraphicsResources();
}

Compositor *Compositor::instance()
{
    return s_compositor();
}

bool Compositor::isRunning() const
{
    Q_D(const Compositor);
    return d->running;
}

bool Compositor::isNested() const
{
    Q_D(const Compositor);
    return d->nested;
}

QString Compositor::shell() const
{
    Q_D(const Compositor);
    return d->shell;
}

QString Compositor::fakeScreenConfiguration() const
{
    Q_D(const Compositor);
    return d->fakeScreenConfiguration;
}

QQmlEngine *Compositor::engine() const
{
    Q_D(const Compositor);
    return d->engine;
}

Compositor::State Compositor::state() const
{
    Q_D(const Compositor);
    return d->state;
}

void Compositor::setState(Compositor::State state)
{
    Q_D(Compositor);

    if (state == d->state)
        return;

    switch (state) {
    case Compositor::Active:
        d->dpms(true);
        Q_EMIT wake();
        d->idleInhibit = 0;
        d->idleTimer->start();
        break;
    case Compositor::Idle:
        d->idleInhibit = 0;
        d->idleTimer->stop();
        d->dpms(false);
        Q_EMIT idle();
        break;
    default:
        break;
    }

    d->state = state;
    Q_EMIT stateChanged();
}

int Compositor::idleInterval() const
{
    Q_D(const Compositor);
    return d->idleInterval;
}

void Compositor::setIdleInterval(int value)
{
    Q_D(Compositor);

    if (d->idleTimer->interval() != value) {
        d->idleInterval = value;
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
    addGlobalInterface(new GreenIslandScreenshooterGlobal());
    addGlobalInterface(new GreenIslandApps());
    addGlobalInterface(new GreenIslandWindows());
    addGlobalInterface(new WlShellGlobal());
    addGlobalInterface(new XdgShellGlobal(this));
    addGlobalInterface(new GtkShellGlobal());
    //addGlobalInterface(new WlSubCompositorGlobal());

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

SurfaceView *Compositor::pickView(const QPointF &globalPosition) const
{
    Q_D(const Compositor);

    // TODO: Views should probably ordered by z-index in order to really
    // pick the first view with that global coordinates

    Q_FOREACH (Output *output, m_compositor->outputs()) {
        if (output->geometry().contains(globalPosition.toPoint())) {
            Q_FOREACH (Surface *surface, output->surfaces()) {
                Q_FOREACH (ClientWindow *window, d->clientWindowsList) {
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

SurfaceItem *Compositor::firstViewOf(Surface *surface)
{
    if (!surface) {
        qCWarning(GREENISLAND_COMPOSITOR) << "First view of null surface requested!";
        return Q_NULLPTR;
    }

    return static_cast<SurfaceItem *>(surface->views().first());
}

SurfaceItem *Compositor::subSurfaceForOutput(Surface *surface, Output *output) const
{
    Q_FOREACH (SurfaceInterface *interface, surface->interfaces()) {
        if (typeid(*interface) != typeid(WlSubSurface))
            continue;
        WlSubSurface *subSurfaceInterface = static_cast<WlSubSurface *>(interface);
        if (subSurfaceInterface)
            return subSurfaceInterface->viewForOutput(output);
    }

    return Q_NULLPTR;
}

void Compositor::surfaceCreated(Surface *surface)
{
    Q_D(Compositor);

    // Ignore cursor surface: we just need its buffer to retrieve
    // the image, we don't really need to do anything but that
    if (surface == d->cursorSurface)
        return;

    qCDebug(GREENISLAND_COMPOSITOR) << "Surface created" << surface;

    Q_EMIT newSurfaceCreated(surface);

    // Connect surface signals
    connect(surface, &Surface::mapped, this, [this, surface] {
        Q_EMIT surfaceMapped(surface);
    });
    connect(surface, &Surface::unmapped, this, [this, surface] {
        Q_EMIT surfaceUnmapped(surface);
    });
    connect(surface, &Surface::surfaceDestroyed, this, [this, surface] {
        Q_EMIT surfaceDestroyed(surface);
    });
}

SurfaceView *Compositor::createView(Surface *surf)
{
    return new WindowView(qobject_cast<QuickSurface *>(surf));
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

void Compositor::unsetMouseCursor()
{
    Q_D(Compositor);

    if (d->clientData.cursorTheme)
        d->clientData.cursorTheme->changeCursor(Client::WlCursorTheme::BlankCursor);
}

void Compositor::resetMouseCursor()
{
    Q_D(Compositor);

    if (d->clientData.cursorTheme)
        d->clientData.cursorTheme->changeCursor(Client::WlCursorTheme::ArrowCursor);
}

void Compositor::abortSession()
{
    QGuiApplication::quit();
}

void Compositor::setCursorSurface(Surface *surface, int hotspotX, int hotspotY)
{
//#if (defined QT_COMPOSITOR_WAYLAND_GL) && (QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 4, 2))
#if 0
    Q_D(Compositor);

    // Do not change the cursor if the compositor wants to
    // exclusively change it (for example during a window move operation)
    if (surface) {
        if (d->cursorGrabbed != WlCursorTheme::BlankCursor && surface->client()->client() != d->clientData.client)
            return;
    }

    // Disconnect configure from the old cursor surface
    if (d->cursorSurface && d->cursorSurface != surface)
        disconnect(d->cursorSurface, SIGNAL(configure(bool)));

    // Update cursor when mapped
    if ((d->cursorSurface != surface) && surface) {
        connect(surface, &Surface::configure, this, [this, d](bool hasBuffer) {
            if (!d->cursorSurface || !hasBuffer)
                return;

            QImage image = static_cast<CursorBufferAttacher *>(d->cursorSurface->bufferAttacher())->image();
            if (image.isNull())
                return;

            QCursor cursor(QPixmap::fromImage(image), d->cursorHotspotX, d->cursorHotspotY);
            if (d->cursorIsSet) {
                QGuiApplication::changeOverrideCursor(cursor);
            } else {
                QGuiApplication::setOverrideCursor(cursor);
                d->cursorIsSet = true;
            }
        });
    }

    // Setup cursor
    d->cursorSurface = surface;
    d->cursorHotspotX = hotspotX;
    d->cursorHotspotY = hotspotY;

    // Buffer attacher
    if (d->cursorSurface && !d->cursorSurface->bufferAttacher())
        d->cursorSurface->setBufferAttacher(new CursorBufferAttacher());
#else
    Q_UNUSED(surface)
    Q_UNUSED(hotspotX)
    Q_UNUSED(hotspotY)
#endif
}

}

#include "moc_compositor.cpp"
