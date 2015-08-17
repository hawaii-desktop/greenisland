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

#ifndef COMPOSITOR_P_H
#define COMPOSITOR_P_H

#include <QtQml/QQmlListProperty>
#include "surface.h"

#include "applicationmanager.h"
#include "compositorsettings.h"
#include "keybinding.h"
#include "screenmanager.h"
#include "client/wlcursortheme.h"

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

struct wl_compositor;

namespace GreenIsland {

namespace Client {
class WlClientConnection;
class WlSeat;
class WlShmPool;
}

class AbstractPlugin;
class Compositor;
class ClientWindow;
class ShellWindow;
class FullScreenShellClient;
class GreenIslandRecorderManager;

class CompositorPrivate
{
public:
    CompositorPrivate(Compositor *self);
    ~CompositorPrivate();

    QQmlListProperty<ClientWindow> windows();
    QQmlListProperty<ShellWindow> shellWindows();

    void loadPlugins();

    void dpms(bool on);

    void grabCursor(Client::WlCursorTheme::CursorShape shape);
    void ungrabCursor();

    void addWindow(ClientWindow *window);
    void removeWindow(ClientWindow *window);

    void mapWindow(ClientWindow *window);
    void unmapWindow(ClientWindow *window);
    void destroyWindow(ClientWindow *window);

    void mapShellWindow(ShellWindow *window);
    void unmapShellWindow(ShellWindow *window);
    void destroyShellWindow(ShellWindow *window);

    void _q_createNestedConnection();
    void _q_createInternalConnection();

    bool running;

    // Compositor
    QString shell;
    Compositor::State state;
    QTimer *idleTimer;
    int idleInterval;
    int idleInhibit;
    QQmlEngine *engine;

    // Cursor
    Surface *cursorSurface;
    int cursorHotspotX;
    int cursorHotspotY;
    Client::WlCursorTheme::CursorShape cursorGrabbed;
    bool cursorIsSet;

    // Keyboard
    Surface *lastKeyboardFocus;

    QString fakeScreenConfiguration;
    CompositorSettings *settings;
    ScreenManager *screenManager;
    GreenIslandRecorderManager *recorderManager;

    // Nested mode
    bool nested;
    Client::WlClientConnection *nestedConnection;
    FullScreenShellClient *fullscreenShell;

    // Client
    struct WlClientData {
        wl_client *client;
        Client::WlClientConnection *connection;
        wl_compositor *compositor;
        Client::WlSeat *seat;
        Client::WlShmPool *shmPool;
        Client::WlCursorTheme *cursorTheme;
    };
    WlClientData clientData;

    // Application management
    QHash<QString, SurfaceList> appSurfaces;
    QList<ClientWindow *> clientWindowsList;
    QList<ShellWindow *> shellWindowsList;

    // Plugins
    QList<AbstractPlugin *> plugins;

protected:
    Q_DECLARE_PUBLIC(Compositor)
    Compositor *const q_ptr;
};

}

#endif // COMPOSITOR_P_H

