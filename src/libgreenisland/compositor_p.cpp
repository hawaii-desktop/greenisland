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

#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QTimer>
#include <QtGui/QGuiApplication>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtCompositor/private/qwlcompositor_p.h>

#include "abstractplugin.h"
#ifdef QT_COMPOSITOR_WAYLAND_GL
#  include "bufferattacher.h"
#endif
#include "compositor.h"
#include "compositor_p.h"
#include "clientwindow.h"
#include "logging.h"
#include "shellwindow.h"
#include "client/wlclientconnection.h"
#include "client/wlregistry.h"
#include "client/wlseat.h"
#include "client/wlshmpool.h"
#include "protocols/fullscreen-shell/fullscreenshellclient.h"

#include <wayland-server.h>

#include <sys/types.h>
#include <sys/socket.h>

namespace GreenIsland {

CompositorPrivate::CompositorPrivate(Compositor *self)
    : running(false)
    , state(Compositor::Active)
    , idleTimer(new QTimer())
    , idleInhibit(0)
    , locked(false)
    , cursorSurface(Q_NULLPTR)
    , cursorHotspotX(0)
    , cursorHotspotY(0)
    , cursorGrabbed(WlCursorTheme::BlankCursor)
    , lastKeyboardFocus(Q_NULLPTR)
    , recorderManager(Q_NULLPTR)
    , nestedConnection(Q_NULLPTR)
    , fullscreenShell(Q_NULLPTR)
    , q_ptr(self)
{
    // Are we nested into another compositor?
    nested = QGuiApplication::platformName().startsWith(QStringLiteral("wayland"));

    idleTimer->setInterval(5 * 60000);

    settings = new CompositorSettings(self);
    screenManager = new ScreenManager(self);

    ApplicationManager::instance();

    // Wayland client connections
    if (nested)
        nestedConnection = new WlClientConnection(self);
    clientData.connection = new WlClientConnection(self);
    clientData.client = Q_NULLPTR;
    clientData.compositor = Q_NULLPTR;
    clientData.seat = Q_NULLPTR;
    clientData.shmPool = Q_NULLPTR;
    clientData.cursorTheme = Q_NULLPTR;
}

CompositorPrivate::~CompositorPrivate()
{
    if (clientData.client)
        wl_client_destroy(clientData.client);

    idleTimer->deleteLater();

    while (!plugins.isEmpty())
        plugins.takeFirst()->deleteLater();

    screenManager->deleteLater();
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

void CompositorPrivate::loadPlugins()
{
    Q_Q(Compositor);

    // We rely on an environment variable that tells us what plugins to load,
    // if it's empty there's no point
    if (qEnvironmentVariableIsEmpty("GREENISLAND_PLUGINS"))
        return;

    QStringList pluginsToLoad = QString::fromUtf8(qgetenv("GREENISLAND_PLUGINS")).split(':');

    const QStringList paths = QCoreApplication::libraryPaths();
    qCDebug(GREENISLAND_COMPOSITOR) << "Lookup paths:" << qPrintable(paths.join(' '));

    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QStringLiteral("/greenisland/"),
                       QStringLiteral("*.so"),
                       QDir::SortFlags(QDir::QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList infoList = dir.entryInfoList();

        Q_FOREACH (const QFileInfo &info, infoList) {
            // Load only the plugins from the environment variable
            if (!pluginsToLoad.contains(info.fileName()))
                continue;

            qCDebug(GREENISLAND_COMPOSITOR) << "Trying" << info.filePath();
            QPluginLoader loader(info.filePath());
            loader.load();

            QObject *instance = loader.instance();
            if (!instance) {
                qCWarning(GREENISLAND_COMPOSITOR, "Plugin loading failed: %s",
                          qPrintable(loader.errorString()));
                loader.unload();
                continue;
            }

            AbstractPlugin *plugin = qobject_cast<AbstractPlugin *>(instance);
            if (plugin) {
                qCDebug(GREENISLAND_COMPOSITOR) << "Loading" << plugin->name() << "plugin";
                plugin->registerPlugin(q);
                plugins.append(plugin);
            } else {
                qCWarning(GREENISLAND_COMPOSITOR,
                          "Plugin instantiation failed: \"%s\" doesn't provide a valid plugin",
                          qPrintable(info.fileName()));
            }
        }
    }
}

void CompositorPrivate::dpms(bool on)
{
    // TODO
    Q_UNUSED(on);
}

void CompositorPrivate::grabCursor(WlCursorTheme::CursorShape shape)
{
    if (clientData.cursorTheme && cursorGrabbed != shape) {
        cursorGrabbed = shape;
        clientData.cursorTheme->changeCursor(cursorGrabbed);
    }
}

void CompositorPrivate::ungrabCursor()
{
    if (clientData.cursorTheme) {
        cursorGrabbed = WlCursorTheme::BlankCursor;
        clientData.cursorTheme->changeCursor(cursorGrabbed);
    }
}

void CompositorPrivate::setCursorImage(const QImage &image)
{
    QCursor cursor(QPixmap::fromImage(image), cursorHotspotX, cursorHotspotY);

    static bool cursorIsSet = false;
    if (cursorIsSet) {
        QGuiApplication::changeOverrideCursor(cursor);
    } else {
        QGuiApplication::setOverrideCursor(cursor);
        cursorIsSet = true;
    }
}

void CompositorPrivate::_q_createNestedConnection()
{
    Q_Q(Compositor);

    Q_ASSERT(nested);

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (!native)
        qFatal("Platform native interface not found, aborting...");

    wl_display *display = static_cast<wl_display *>(
                native->nativeResourceForIntegration("display"));
    if (!display)
        qFatal("Wayland connection is not available, aborting...");

    nestedConnection->setDisplay(display);

    q->connect(nestedConnection, &WlClientConnection::connected, q, [this, q, display] {
        WlRegistry *registry = new WlRegistry(nestedConnection);
        registry->create(nestedConnection->display());

        q->connect(registry, &WlRegistry::fullscreenShellAnnounced, q,
                [this, registry](quint32 name, quint32 version) {
            // Create fullscreen shell client
            fullscreenShell = new FullScreenShellClient(registry->registry(), name, version);

            // Create outputs only now so that OutputWindow will be able to
            // show itself on fullscreen shell
            QMetaObject::invokeMethod(screenManager, "acquireConfiguration",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, fakeScreenConfiguration));
        });
        q->connect(registry, &WlRegistry::fullscreenShellRemoved, q, [this](quint32) {
            // Delete fullscreen shell client
            delete fullscreenShell;
            fullscreenShell = Q_NULLPTR;
        });

        registry->setup();

        // Queue internal connection initialization
        QMetaObject::invokeMethod(q, "_q_createInternalConnection",
                                  Qt::QueuedConnection);
    });

    // Initialize the client connection
    nestedConnection->initializeConnection();
}

void CompositorPrivate::_q_createInternalConnection()
{
    Q_Q(Compositor);

    int sockets[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets) < 0) {
        qCWarning(GREENISLAND_COMPOSITOR) << "Unable to create sockets for internal connection";
        return;
    }

    // The only reason why we need to connect to ourselves is
    // wayland-cursor: in order to set a cursor for move and resize operations
    // we need to load the image from the theme but wayland-cursor
    // requires a wl_shm which is only available to Wayland clients
    clientData.client = wl_client_create(q->handle()->wl_display(), sockets[0]);
    clientData.connection->setSocketFd(sockets[1]);
    q->connect(clientData.connection, &WlClientConnection::connected, q, [this, q] {
        WlRegistry *registry = new WlRegistry(clientData.connection);
        registry->create(clientData.connection->display());

        q->connect(registry, &WlRegistry::compositorAnnounced, q,
                   [this, registry, q](quint32, quint32) {
            // Bind to the compositor
            clientData.compositor = registry->bindCompositor();
        });
        q->connect(registry, &WlRegistry::compositorRemoved, q,
                   [this, registry](quint32) {
            // Delete compositor
            if (clientData.compositor) {
                wl_compositor_destroy(clientData.compositor);
                clientData.compositor = Q_NULLPTR;
            }
        });

        q->connect(registry, &WlRegistry::seatAnnounced, q,
                   [this, registry, q](quint32 name, quint32 version) {
            // Bind to the seat
            clientData.seat = new WlSeat(registry, clientData.compositor,
                                         name, version);
        });
        q->connect(registry, &WlRegistry::seatRemoved, q,
                   [this, registry](quint32) {
            // Delete seat
            delete clientData.seat;
            clientData.seat = Q_NULLPTR;
        });

        q->connect(registry, &WlRegistry::shmAnnounced, q,
                   [this, registry, q](quint32, quint32) {
            // Create a shared memory pool
            clientData.shmPool = registry->createShmPool(clientData.connection);
        });
        q->connect(registry, &WlRegistry::shmRemoved, q,
                   [this, registry](quint32) {
            // Delete shared memory pool
            if (clientData.shmPool)
                clientData.shmPool->deleteLater();
            clientData.shmPool = Q_NULLPTR;
        });

        q->connect(registry, &WlRegistry::interfacesAnnounced, q, [this] {
            // All interfaces were announced, create the cursor theme
            clientData.cursorTheme = new WlCursorTheme(clientData.shmPool, clientData.seat);
        });
        q->connect(registry, &WlRegistry::interfacesRemoved, q, [this] {
            // All interfaces are gone so shmpool and seat are not valid anymore,
            // delete the cursor theme
            delete clientData.cursorTheme;
            clientData.cursorTheme = Q_NULLPTR;
        });

        registry->setup();
    });

    // Initialize the client connection
    clientData.connection->initializeConnection();
}

void CompositorPrivate::_q_updateCursor(bool hasBuffer)
{
    if (!hasBuffer || !cursorSurface || !cursorSurface->bufferAttacher())
        return;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QImage image = static_cast<BufferAttacher *>(cursorSurface->bufferAttacher())->image();
    setCursorImage(image);
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

    Q_EMIT ApplicationManager::instance()->windowMapped(window);
    Q_EMIT q->windowMapped(window);
}

void CompositorPrivate::unmapWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    removeWindow(window);

    Q_EMIT ApplicationManager::instance()->windowUnmapped(window);
    Q_EMIT q->windowUnmapped(window);
}

void CompositorPrivate::destroyWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    removeWindow(window);

    Q_EMIT ApplicationManager::instance()->windowUnmapped(window);
    Q_EMIT q->windowDestroyed(window->id());
}

void CompositorPrivate::mapShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (!shellWindowsList.contains(window)) {
        shellWindowsList.append(window);
        Q_EMIT q->shellWindowMapped(window);
        Q_EMIT q->shellWindowsChanged();
    }
}

void CompositorPrivate::unmapShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (shellWindowsList.removeOne(window)) {
        Q_EMIT q->shellWindowUnmapped(window);
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

}
