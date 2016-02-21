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

#ifndef GREENISLAND_QUICKXDGPOPUPITEM_H
#define GREENISLAND_QUICKXDGPOPUPITEM_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>
#include <GreenIsland/Server/XdgShell>

namespace GreenIsland {

namespace Server {

class QuickXdgPopupItemPrivate;

class GREENISLANDSERVER_EXPORT QuickXdgPopupItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuickXdgPopupItem)
    Q_PROPERTY(XdgPopup *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
public:
    QuickXdgPopupItem(QQuickItem *parent = 0);

    XdgPopup *shellSurface() const;
    void setShellSurface(XdgPopup *shellSurface);

    static QuickXdgPopupItemPrivate *get(QuickXdgPopupItem *item) { return item->d_func(); }

Q_SIGNALS:
    void shellSurfaceChanged();

protected:
    QuickXdgPopupItem(QuickXdgPopupItemPrivate &dd, QQuickItem *parent);

    void surfaceChangedEvent(QWaylandSurface *newSurface,
                             QWaylandSurface *oldSurface) Q_DECL_OVERRIDE;

    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void componentComplete() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void handleWindowChanged(QQuickWindow *window);
    void adjustOffsetForNextFrame(const QPointF &offset);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_QUICKXDGPOPUPITEM_H
