/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL3GPL2$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPLv2 included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
