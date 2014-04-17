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

#ifndef GREENISLANDCOMPOSITOR_H
#define GREENISLANDCOMPOSITOR_H

#include <QtCore/QLoggingCategory>
#include <QtCore/QProcess>
#include <QtQuick/QQuickView>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandSurface>

#include <GreenIsland/GreenIslandGlobal>

Q_DECLARE_LOGGING_CATEGORY(GREENISLAND_COMPOSITOR)

namespace GreenIsland {

class CompositorPrivate;

class GREENISLAND_EXPORT Compositor : public QQuickView, public QWaylandCompositor
{
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString shellFileName READ shellFileName WRITE setShellFileName NOTIFY shellFileNameChanged)
    Q_PROPERTY(bool shellClientRunning READ isShellClientRunning NOTIFY shellClientRunningChanged)
    Q_ENUMS(State)
    Q_DECLARE_PRIVATE(Compositor)
public:
    enum State {
        //! Compositor is active.
        CompositorActive,
        //! Shell unlock called on activity.
        CompositorIdle,
        //! No rendering, no frame events.
        CompositorOffscreen,
        //! Same as CompositorOffscreen, but also set DPMS
        CompositorSleeping
    };

    explicit Compositor(const char *socketName = 0,
                        ExtensionFlags extensions = DefaultExtensions | SubSurfaceExtension);
    ~Compositor();

    State state() const;
    void setState(Compositor::State state);

    QString shellFileName() const;
    void setShellFileName(const QString &fileName);

    void showGraphicsInfo();

    void runShell(const QStringList &arguments = QStringList());
    void closeShell();

    bool isShellClientRunning() const;

    quint32 idleInterval() const;
    void setIdleInterval(quint32 msecs);

    Q_INVOKABLE void startIdleTimer();
    Q_INVOKABLE void stopIdleTimer();

    virtual void surfaceCreated(QWaylandSurface *surface);
    virtual void surfaceAboutToBeDestroyed(QWaylandSurface *surface);

public Q_SLOTS:
    void damageAll();

Q_SIGNALS:
    void stateChanged(Compositor::State state);
    void shellFileNameChanged(const QString &fileName);
    void shellClientRunningChanged(bool value);

    void surfaceMapped(QWaylandSurface *surface);
    void surfaceUnmapped(QWaylandSurface *surface);
    void surfaceDestroyed(QWaylandSurface *surface);

    void ready();

    void idle();
    void wake();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void wheelEvent(QWheelEvent *event);

    virtual void touchEvent(QTouchEvent *event);

    void resizeEvent(QResizeEvent *event);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_startFrame())
    Q_PRIVATE_SLOT(d_func(), void _q_sendCallbacks())

    Q_PRIVATE_SLOT(d_func(), void _q_shellStarted())
    Q_PRIVATE_SLOT(d_func(), void _q_shellFailed(QProcess::ProcessError error))
    Q_PRIVATE_SLOT(d_func(), void _q_shellReadyReadStandardOutput())
    Q_PRIVATE_SLOT(d_func(), void _q_shellReadyReadStandardError())
    Q_PRIVATE_SLOT(d_func(), void _q_shellAboutToClose())

    CompositorPrivate *const d_ptr;
};

}

#endif // GREENISLANDCOMPOSITOR_H
