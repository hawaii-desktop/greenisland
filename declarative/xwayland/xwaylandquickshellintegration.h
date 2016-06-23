/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#ifndef XWAYLANDQUICKSHELLINTEGRATION_H
#define XWAYLANDQUICKSHELLINTEGRATION_H

#include <QtCore/QObject>
#include <QtGui/QMouseEvent>

#include "xwaylandshellsurface.h"

class XWaylandQuickShellSurfaceItem;

class XWaylandQuickShellIntegration : public QObject
{
    Q_OBJECT
public:
    XWaylandQuickShellIntegration(XWaylandQuickShellSurfaceItem *item);

    bool mouseMoveEvent(QMouseEvent *event);
    bool mouseReleaseEvent(QMouseEvent *event);

private Q_SLOTS:
    void handleStartMove();
    void handleStartResize(XWaylandShellSurface::ResizeEdge edges);

private:
    XWaylandQuickShellSurfaceItem *m_item;
    XWaylandShellSurface *m_shellSurface;

    enum class GrabberState {
        Default,
        Resize,
        Move
    };

    GrabberState grabberState;

    struct {
        QPointF initialOffset;
        bool initialized;
    } moveState;

    struct {
        XWaylandShellSurface::ResizeEdge resizeEdges;
        QSizeF initialSize;
        QPointF initialMousePos;
        bool initialized;
    } resizeState;
};

#endif // XWAYLANDQUICKSHELLINTEGRATION_H
