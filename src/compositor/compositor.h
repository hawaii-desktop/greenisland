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

#include <QtQuick/QQuickView>
#include <QtCompositor/QWaylandQuickCompositor>
#include <QtCompositor/QWaylandSurfaceItem>

class CompositorPrivate;
class Shell;
class ShellSurface;
class ClientWindow;
class Workspace;
class Grab;
class PanelManager;
class ScreenModel;
class ScreenSaver;
class Notifications;

class Compositor : public QQuickView, public QWaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(int idleInterval READ idleInterval WRITE setIdleInterval NOTIFY idleIntervalChanged)
    Q_PROPERTY(int idleInhibit READ idleInhibit WRITE setIdleInhibit NOTIFY idleInhibitChanged)
    Q_ENUMS(State WindowRole)
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

    enum WindowRole {
        ApplicationRole,
        CursorRole,
        LockScreenRole,
        OverlayRole,
        DialogRole,
        FullScreenRole,
        PanelRole,
        PopupRole,
        NotificationRole,
        DesktopRole,
        BackgroundRole
    };

    explicit Compositor(const QString &socket = QString());
    ~Compositor();

    State state() const;
    void setState(State state);

    int idleInterval() const;
    void setIdleInterval(int value);

    int idleInhibit() const;
    void setIdleInhibit(int value);

    ScreenModel *screenModel() const;

    Q_INVOKABLE QWaylandSurfaceItem *firstViewOf(QWaylandSurface *surface);

    virtual void surfaceCreated(QWaylandSurface *surface);

    Q_INVOKABLE QPointF calculateInitialPosition(QWaylandSurface *surface);

Q_SIGNALS:
    void surfaceMapped(QVariant surface);
    void surfaceUnmapped(QVariant surface);
    void surfaceDestroyed(QVariant surface);

    void stateChanged();
    void idleIntervalChanged();
    void idleInhibitChanged();

    void idleInhibitResetRequested();
    void idleTimerStartRequested();
    void idleTimerStopRequested();

    void idle();
    void wake();

    void ready();

    void fadeIn();
    void fadeOut();

    void locked();
    void unlocked();

    void workspaceAdded();
    void workspaceRemoved(int index);
    void workspaceSelected(int index);

public Q_SLOTS:
    void lockSession();
    void unlockSession();

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void resizeEvent(QResizeEvent *event);

    void setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY);

private:
    Q_DECLARE_PRIVATE(Compositor)
    CompositorPrivate *const d_ptr;

private:
#if 0
    QList<ClientWindow *> m_clientWindows;
    QList<Workspace *> m_workspaces;
#endif

    Q_PRIVATE_SLOT(d_func(), void _q_resizeCompositor())
    Q_PRIVATE_SLOT(d_func(), void _q_sendCallbacks())
    Q_PRIVATE_SLOT(d_func(), void _q_updateCursor(bool hasBuffer))
    Q_PRIVATE_SLOT(d_func(), void _q_surfaceDestroyed(QObject *object))
    Q_PRIVATE_SLOT(d_func(), void _q_surfaceMapped())
    Q_PRIVATE_SLOT(d_func(), void _q_surfaceUnmapped())
};

#endif // COMPOSITOR_H
