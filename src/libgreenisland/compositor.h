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

class ApplicationManager;
class ClientWindow;
class CompositorPrivate;
class Output;
class QuickSurface;
class ScreenManager;

class GREENISLAND_EXPORT Compositor : public QObject, public QWaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(ApplicationManager *applicationManager READ applicationManager CONSTANT)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(int idleInterval READ idleInterval WRITE setIdleInterval NOTIFY idleIntervalChanged)
    Q_PROPERTY(int idleInhibit READ idleInhibit WRITE setIdleInhibit NOTIFY idleInhibitChanged)
    Q_PROPERTY(QQmlListProperty<ClientWindow> windows READ windows)
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

    explicit Compositor(const QString &socket = QString());
    ~Compositor();

    State state() const;
    void setState(State state);

    int idleInterval() const;
    void setIdleInterval(int value);

    int idleInhibit() const;
    void setIdleInhibit(int value);

    ScreenManager *screenManager() const;
    ApplicationManager *applicationManager() const;

    void run();

    QWaylandSurface *createSurface(QWaylandClient *client, quint32 id, int version);

    Q_INVOKABLE QWaylandSurfaceView *pickView(const QPointF &globalPosition) const Q_DECL_OVERRIDE;

    Q_INVOKABLE QWaylandSurfaceItem *firstViewOf(QuickSurface *surface);
    Q_INVOKABLE QWaylandSurfaceItem *viewForOutput(QuickSurface *surface, Output *output);

    virtual void surfaceCreated(QWaylandSurface *surface);

    Q_INVOKABLE QPointF calculateInitialPosition(QWaylandSurface *surface);

    Q_INVOKABLE void abortSession();

    QQmlListProperty<ClientWindow> windows();
    int windowCount() const;
    ClientWindow *window(int) const;

    QList<ClientWindow *> windowsList() const;

    static QString s_fixedPlugin;

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

protected:
    void setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY);

private:
    Q_DECLARE_PRIVATE(Compositor)
    CompositorPrivate *const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_updateCursor(bool hasBuffer))
};

}

#endif // COMPOSITOR_H
