/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef WAYLANDWINDOWITEM_H
#define WAYLANDWINDOWITEM_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>

class WaylandWindowItemPrivate;

class WaylandWindowItem : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWindowItem)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandQuickItem *shellSurfaceItem READ shellSurfaceItem WRITE setShellSurfaceItem NOTIFY shellSurfaceItemChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool minimized READ isMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullScreen READ isFullScreen NOTIFY fullScreenChanged)
public:
    enum Type {
        UnknownWindowType = 0,
        TopLevelWindowType,
        TransientWindowType,
        PopupWindowType
    };
    Q_ENUM(Type)

    WaylandWindowItem(QQuickItem *parent = Q_NULLPTR);

    QWaylandSurface *surface() const;

    QWaylandQuickItem *shellSurfaceItem() const;
    void setShellSurfaceItem(QWaylandQuickItem *item);

    QString title() const;
    QString appId() const;

    bool isActive() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isFullScreen() const;

    Q_INVOKABLE QPointF randomPosition() const;

Q_SIGNALS:
    void surfaceChanged();
    void shellSurfaceItemChanged();
    void titleChanged();
    void appIdChanged();
    void activeChanged();
    void minimizedChanged();
    void maximizedChanged();
    void fullScreenChanged();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void lower();
    void raise();

private Q_SLOTS:
    void shellSurfaceItemWidthChanged();
    void shellSurfaceItemHeightChanged();
    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setActive(bool active);
    void setMinimized(bool minimized);
    void setMaximized(bool maximized);
    void setFullScreen(bool fullScreen);
};

#endif // WAYLANDWINDOWITEM_H
