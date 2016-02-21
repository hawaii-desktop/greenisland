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

