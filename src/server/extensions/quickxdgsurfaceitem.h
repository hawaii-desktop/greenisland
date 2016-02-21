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

#ifndef GREENISLAND_QUICKXDGSURFACEITEM_H
#define GREENISLAND_QUICKXDGSURFACEITEM_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>
#include <GreenIsland/Server/XdgShell>

namespace GreenIsland {

namespace Server {

class QuickXdgSurfaceItemPrivate;

class GREENISLANDSERVER_EXPORT QuickXdgSurfaceItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuickXdgSurfaceItem)
    Q_PROPERTY(XdgSurface *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)
public:
    QuickXdgSurfaceItem(QQuickItem *parent = 0);

    XdgSurface *shellSurface() const;
    void setShellSurface(XdgSurface *shellSurface);

    QQuickItem *moveItem() const;
    void setMoveItem(QQuickItem *moveItem);

    static QuickXdgSurfaceItemPrivate *get(QuickXdgSurfaceItem *item) { return item->d_func(); }

Q_SIGNALS:
    void shellSurfaceChanged();
    void shelPopupChanged();
    void moveItemChanged();

protected:
    QuickXdgSurfaceItem(QuickXdgSurfaceItemPrivate &dd, QQuickItem *parent);

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void surfaceChangedEvent(QWaylandSurface *newSurface,
                             QWaylandSurface *oldSurface) Q_DECL_OVERRIDE;

    void componentComplete() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void handleStartMove(QWaylandInputDevice *inputDevice);
    void handleStartResize(QWaylandInputDevice *inputDevice,
                           XdgSurface::ResizeEdge edge);
    void adjustOffsetForNextFrame(const QPointF &offset);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_QUICKXDGSURFACEITEM_H
