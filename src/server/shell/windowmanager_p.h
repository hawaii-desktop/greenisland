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

#ifndef GREENISLAND_WINDOWMANAGER_P_H
#define GREENISLAND_WINDOWMANAGER_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Server/WindowManager>

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

class ClientWindow;

class GREENISLANDSERVER_EXPORT WindowManagerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(WindowManager)
public:
    WindowManagerPrivate()
        : compositor(Q_NULLPTR)
        , rootItem(new QQuickItem())
    {}

    ~WindowManagerPrivate()
    {
        delete rootItem;
    }

    QQmlListProperty<ClientWindow> windows();

    void registerWindow(ClientWindow *window);
    void unregisterWindow(ClientWindow *window);

    static int windowsCount(QQmlListProperty<ClientWindow> *prop);
    static ClientWindow *windowsAt(QQmlListProperty<ClientWindow> *prop, int index);

    static WindowManagerPrivate *get(WindowManager *wm) { return wm->d_func(); }

    QWaylandCompositor *compositor;
    QQuickItem *rootItem;
    QVector<ClientWindow *> windowsList;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_WINDOWMANAGER_P_H
