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

#ifndef XWAYLANDQUICKSHELLSURFACEITEM_H
#define XWAYLANDQUICKSHELLSURFACEITEM_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>

class XWaylandQuickShellIntegration;
class XWaylandShellSurface;

class XWaylandQuickShellSurfaceItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_PROPERTY(XWaylandShellSurface *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)
public:
    XWaylandQuickShellSurfaceItem(QQuickItem *parent = nullptr);

    XWaylandShellSurface *shellSurface() const;
    void setShellSurface(XWaylandShellSurface *shellSurface);

    QQuickItem *moveItem() const;
    void setMoveItem(QQuickItem *moveItem);

Q_SIGNALS:
    void shellSurfaceChanged();
    void moveItemChanged();

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    XWaylandShellSurface *m_shellSurface;
    XWaylandQuickShellIntegration *m_shellIntegration;
    QQuickItem *m_moveItem;
};

#endif // XWAYLANDQUICKSHELLSURFACEITEM_H
