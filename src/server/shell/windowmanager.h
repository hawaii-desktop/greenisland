/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLAND_WINDOWMANAGER_H
#define GREENISLAND_WINDOWMANAGER_H

#include <GreenIsland/QtWaylandCompositor/QWaylandShellSurface>

#include <GreenIsland/Server/ClientWindow>

class QWaylandClient;
class QWaylandCompositor;
class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class WindowManagerPrivate;

class GREENISLANDSERVER_EXPORT WindowManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WindowManager)
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    Q_PRIVATE_PROPERTY(WindowManager::d_func(), QQmlListProperty<GreenIsland::Server::ClientWindow> windows READ windows NOTIFY windowsChanged)
public:
    WindowManager(QObject *parent = Q_NULLPTR);
    WindowManager(QWaylandCompositor *compositor, QObject *parent = Q_NULLPTR);

    QWaylandCompositor *compositor() const;
    void setCompositor(QWaylandCompositor *compositor);

    Q_INVOKABLE QVariantList windowsForOutput(QWaylandOutput *desiredOutput = Q_NULLPTR) const;

    Q_INVOKABLE void recalculateVirtualGeometry();

    Q_INVOKABLE void initialize();

Q_SIGNALS:
    void compositorChanged();
    void windowCreated(ClientWindow *window);
    void windowsChanged();

private Q_SLOTS:
    void createWlShellSurface(QWaylandSurface *surface,
                              QWaylandClient *client, uint id);
    void createXdgSurface(QWaylandSurface *surface,
                          QWaylandClient *client, uint id);
    void createXdgPopup(QWaylandInputDevice *inputDevice,
                        QWaylandSurface *surface, QWaylandSurface *parentSurface,
                        const QPoint &relativeToParent,
                        QWaylandClient *client, uint id);
    void createGtkShellSurface(QWaylandSurface *surface,
                               QWaylandClient *client, uint id);

    void setAppId();
    void setTitle();
    void setXdgWindowGeometry();
    void setTopLevel();
    void setWlTransient(QWaylandSurface *parentSurface,
                        const QPoint &relativeToParent,
                        QWaylandShellSurface::FocusPolicy policy);
    void setWlPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parentSurface,
                    const QPoint &relativeToParent);
    void setXdgTransient(QWaylandSurface *parentSurface);
    void setMaximized(QWaylandOutput *output);
    void setWlFullScreen(QWaylandShellSurface::FullScreenMethod method,
                         uint framerate, QWaylandOutput *output);
    void setXdgFullScreen(QWaylandOutput *output);

    void surfaceDestroyed();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_WINDOWMANAGER_H
