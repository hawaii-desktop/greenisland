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
