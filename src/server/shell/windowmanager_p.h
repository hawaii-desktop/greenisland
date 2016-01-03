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

#ifndef GREENISLAND_WINDOWMANAGER_P_H
#define GREENISLAND_WINDOWMANAGER_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/QtWaylandCompositor/QWaylandShell>

#include <GreenIsland/Server/WindowManager>
#include <GreenIsland/Server/XdgShell>
#include <GreenIsland/Server/GtkShell>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace GreenIsland {

namespace Server {

class ApplicationManager;
class ClientWindow;

class GREENISLANDSERVER_EXPORT WindowManagerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(WindowManager)
public:
    WindowManagerPrivate()
        : initialized(false)
        , compositor(Q_NULLPTR)
        , rootItem(new QQuickItem())
        , wlShell(Q_NULLPTR)
        , xdgShell(Q_NULLPTR)
        , gtkShell(Q_NULLPTR)
        , appMan(Q_NULLPTR)
    {}

    ~WindowManagerPrivate()
    {
        delete rootItem;
    }

    ClientWindow *windowForSurface(QWaylandSurface *surface) const;

    QQmlListProperty<ClientWindow> windows();

    static int windowsCount(QQmlListProperty<ClientWindow> *prop);
    static ClientWindow *windowsAt(QQmlListProperty<ClientWindow> *prop, int index);

    static WindowManagerPrivate *get(WindowManager *wm) { return wm->d_func(); }

    bool initialized;
    QWaylandCompositor *compositor;
    QQuickItem *rootItem;
    QWaylandShell *wlShell;
    XdgShell *xdgShell;
    GtkShell *gtkShell;
    QVector<ClientWindow *> windowsList;
    ApplicationManager *appMan;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_WINDOWMANAGER_P_H
