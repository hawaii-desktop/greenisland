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

#ifndef GREENISLAND_WINDOWMANAGER_H
#define GREENISLAND_WINDOWMANAGER_H

#include <GreenIsland/Server/ClientWindow>

class QWaylandClient;
class QWaylandCompositor;

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

    Q_INVOKABLE GreenIsland::Server::ClientWindow *createWindow(QWaylandSurface *surface);
    Q_INVOKABLE GreenIsland::Server::ClientWindow *windowForSurface(QWaylandSurface *surface) const;

    Q_INVOKABLE QVariantList windowsForOutput(QWaylandOutput *desiredOutput = Q_NULLPTR) const;

    Q_INVOKABLE void recalculateVirtualGeometry();

Q_SIGNALS:
    void compositorChanged();
    void windowsChanged();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_WINDOWMANAGER_H
