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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtCompositor/QWaylandQuickCompositor>
#include <QtCompositor/QWaylandSurfaceItem>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class CompositorSettings;
class ClientWindow;
class CompositorPrivate;
class KeyBinding;
class Output;
class ScreenManager;
class ShellWindow;
class GtkSurface;
class WlCursorTheme;
class WlCursorThemePrivate;
class WlShellSurfaceMoveGrabber;
class WlShellSurfaceResizeGrabber;
class XdgSurfaceMoveGrabber;
class XdgSurfaceResizeGrabber;

class GREENISLAND_EXPORT Compositor : public QObject, public QWaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(CompositorSettings *settings READ settings CONSTANT)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(int idleInterval READ idleInterval WRITE setIdleInterval NOTIFY idleIntervalChanged)
    Q_PROPERTY(int idleInhibit READ idleInhibit WRITE setIdleInhibit NOTIFY idleInhibitChanged)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockedChanged)
    Q_PRIVATE_PROPERTY(Compositor::d_func(), QQmlListProperty<ClientWindow> windows READ windows NOTIFY windowsChanged)
    Q_PRIVATE_PROPERTY(Compositor::d_func(), QQmlListProperty<ShellWindow> shellWindows READ shellWindows NOTIFY shellWindowsChanged)
    Q_ENUMS(State)
public:
    enum State {
        //! Compositor is active.
        Active,
        //! Shell unlock called on activity.
        Idle,
        //! No rendering, no frame events.
        Offscreen,
        //! Same as CompositorOffscreen, but also set DPMS
        Sleeping
    };

    Compositor(const QString &socket = QString());
    ~Compositor();

    void setFakeScreenConfiguration(const QString &fileName);

    State state() const;
    void setState(State state);

    int idleInterval() const;
    void setIdleInterval(int value);

    int idleInhibit() const;
    void setIdleInhibit(int value);

    void incrementIdleInhibit();
    void decrementIdleInhibit();

    bool isLocked() const;
    void setLocked(bool value);

    CompositorSettings *settings() const;

    QList<KeyBinding> keyBindings() const;

    void run();

    Q_INVOKABLE QWaylandSurfaceView *pickView(const QPointF &globalPosition) const Q_DECL_OVERRIDE;
    Q_INVOKABLE QWaylandSurfaceItem *firstViewOf(QWaylandSurface *surface);

    void surfaceCreated(QWaylandSurface *surface) Q_DECL_OVERRIDE;
    QWaylandSurfaceView *createView(QWaylandSurface *surf) Q_DECL_OVERRIDE;

    Q_INVOKABLE void clearKeyboardFocus();
    Q_INVOKABLE void restoreKeyboardFocus();

    Q_INVOKABLE bool registerKeyBinding(const QString &name, const QString &keys);
    Q_INVOKABLE bool unregisterKeyBinding(const QString &name);

    Q_INVOKABLE void abortSession();

    static QString s_fixedShell;

Q_SIGNALS:
    void newSurfaceCreated(QWaylandSurface *surface);
    void surfaceMapped(QWaylandSurface *surface);
    void surfaceUnmapped(QWaylandSurface *surface);
    void surfaceDestroyed(QWaylandSurface *surface);

    void windowMapped(ClientWindow *window);
    void windowUnmapped(ClientWindow *window);
    void windowDestroyed(uint id);

    void shellWindowMapped(ShellWindow *window);
    void shellWindowUnmapped(ShellWindow *window);
    void shellWindowDestroyed(uint id);

    void stateChanged();
    void idleIntervalChanged();
    void idleInhibitChanged();
    void windowsChanged();
    void shellWindowsChanged();

    void lockedChanged();

    void idle();
    void wake();

    void fadeIn();
    void fadeOut();

    void screenConfigurationAcquired();

    void keyBindingPressed(const QString &name);
    void keyBindingReleased(const QString &name);

protected:
    void setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY) Q_DECL_OVERRIDE;

private:
    Q_DECLARE_PRIVATE(Compositor)
    CompositorPrivate *const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_createNestedConnection())
    Q_PRIVATE_SLOT(d_func(), void _q_createInternalConnection())
    Q_PRIVATE_SLOT(d_func(), void _q_updateCursor(bool hasBuffer))

    friend class ClientWindow;
    friend class ScreenManager;
    friend class ShellWindow;
    friend class OutputWindow;
    friend class GtkSurface;
    friend class WlCursorTheme;
    friend class WlCursorThemePrivate;
    friend class WlShellSurfaceMoveGrabber;
    friend class WlShellSurfaceResizeGrabber;
    friend class XdgSurfaceMoveGrabber;
    friend class XdgSurfaceResizeGrabber;
};

}

#endif // COMPOSITOR_H
