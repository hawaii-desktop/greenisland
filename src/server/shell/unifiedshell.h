/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_UNIFIEDSHELL_H
#define GREENISLAND_UNIFIEDSHELL_H

#include <GreenIsland/QtWaylandCompositor/QWaylandWlShellSurface>

#include <GreenIsland/Server/ClientWindow>

class QWaylandClient;
class QWaylandCompositor;
class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class UnifiedShellPrivate;

class GREENISLANDSERVER_EXPORT UnifiedShell : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(UnifiedShell)
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    Q_PRIVATE_PROPERTY(UnifiedShell::d_func(), QQmlListProperty<GreenIsland::Server::ClientWindow> windows READ windows NOTIFY windowsChanged)
public:
    UnifiedShell(QObject *parent = Q_NULLPTR);
    UnifiedShell(QWaylandCompositor *compositor, QObject *parent = Q_NULLPTR);

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
                              const QWaylandResource &resource);
    void createXdgSurface(QWaylandSurface *surface,
                          const QWaylandResource &resource);
    void createXdgPopup(QWaylandInputDevice *inputDevice,
                        QWaylandSurface *surface, QWaylandSurface *parentSurface,
                        const QPoint &relativeToParent,
                        const QWaylandResource &resource);
    void createGtkShellSurface(QWaylandSurface *surface,
                               QWaylandClient *client, uint id);

    void setAppId();
    void setTitle();
    void setWlWindowGeometry();
    void setXdgWindowGeometry();
    void setTopLevel();
    void setWlTransient(QWaylandSurface *parentSurface,
                        const QPoint &relativeToParent,
                        QWaylandWlShellSurface::FocusPolicy policy);
    void setWlPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parentSurface,
                    const QPoint &relativeToParent);
    void setXdgTransient(QWaylandSurface *parentSurface);
    void setMaximized(QWaylandOutput *output);
    void setWlFullScreen(QWaylandWlShellSurface::FullScreenMethod method,
                         uint framerate, QWaylandOutput *output);
    void setXdgFullScreen(QWaylandOutput *output);

    void surfaceDestroyed();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_UNIFIEDSHELL_H
