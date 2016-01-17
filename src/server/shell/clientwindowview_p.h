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

#ifndef CLIENTWINDOWVIEW_P_H
#define CLIENTWINDOWVIEW_P_H

#include <QtQuick/private/qquickitem_p.h>

#include <GreenIsland/Server/ClientWindow>

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

namespace GreenIsland {

namespace Server {

class GREENISLANDSERVER_EXPORT ClientWindowViewPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(ClientWindowView)
public:
    enum GrabberState {
        DefaultState,
        MoveState
    };

    ClientWindowViewPrivate();
    ~ClientWindowViewPrivate();

    void setShellSurfaceItem(QWaylandQuickItem *item);

    bool mousePressEvent(QMouseEvent *event);
    bool mouseReleaseEvent(QMouseEvent *event);
    bool mouseMoveEvent(QMouseEvent *event);

    static ClientWindowViewPrivate *get(ClientWindowView *view) { return view->d_func(); }

    bool initialized;
    QWaylandQuickOutput *output;
    QWaylandQuickItem *shellSurfaceItem;
    QQmlPropertyMap *savedProperties;
    ClientWindow *window;

    GrabberState grabberState;

    struct {
        QPointF initialOffset;
        bool initialized;
    } moveState;
};

} // namespace Server

} // namespace GreenIsland

#endif // CLIENTWINDOWVIEW_P_H

