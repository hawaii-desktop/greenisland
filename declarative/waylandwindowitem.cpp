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

#include <QtGui/QGuiApplication>
#include <QtQuick/private/qquickitem_p.h>

#include "waylandwindowitem.h"

class WaylandWindowItemPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(WaylandWindowItem)
public:
    enum GrabberState {
        DefaultState,
        MoveState
    };

    WaylandWindowItemPrivate()
        : shellSurfaceItem(Q_NULLPTR)
        , active(false)
        , minimized(false)
        , maximized(false)
        , fullScreen(false)
        , grabberState(DefaultState)
    {
    }

    QWaylandQuickItem *shellSurfaceItem;

    QString title;
    QString appId;

    bool active;
    bool minimized;
    bool maximized;
    bool fullScreen;

    GrabberState grabberState;

    struct {
        QPointF initialOffset;
        bool initialized;
    } moveState;
};

WaylandWindowItem::WaylandWindowItem(QQuickItem *parent)
    : QQuickItem(*new WaylandWindowItemPrivate(), parent)
{
    setFlag(ItemClipsChildrenToShape);
    setAcceptedMouseButtons(Qt::AllButtons);
}

QWaylandSurface *WaylandWindowItem::surface() const
{
    Q_D(const WaylandWindowItem);

    if (d->shellSurfaceItem)
        return d->shellSurfaceItem->surface();
    return Q_NULLPTR;
}

QWaylandQuickItem *WaylandWindowItem::shellSurfaceItem() const
{
    Q_D(const WaylandWindowItem);
    return d->shellSurfaceItem;
}

void WaylandWindowItem::setShellSurfaceItem(QWaylandQuickItem *item)
{
    Q_D(WaylandWindowItem);

    if (d->shellSurfaceItem == item)
        return;

    if (d->shellSurfaceItem) {
        disconnect(item, &QWaylandQuickItem::surfaceChanged,
                   this, &WaylandWindowItem::surfaceChanged);
        disconnect(d->shellSurfaceItem, &QWaylandQuickItem::widthChanged,
                   this, &WaylandWindowItem::shellSurfaceItemWidthChanged);
        disconnect(d->shellSurfaceItem, &QWaylandQuickItem::heightChanged,
                   this, &WaylandWindowItem::shellSurfaceItemHeightChanged);

        disconnect(item, SIGNAL(titleChanged(QString)),
                   this, SLOT(setTitle(QString)));
        disconnect(item, SIGNAL(appIdChanged(QString)),
                   this, SLOT(setAppId(QString)));

        disconnect(item, SIGNAL(activeChanged(bool)),
                   this, SLOT(setActive(bool)));
        disconnect(item, SIGNAL(minimizedChanged(bool)),
                   this, SLOT(setMinimized(bool)));
        disconnect(item, SIGNAL(maximizedChanged(bool)),
                   this, SLOT(setMaximized(bool)));
        disconnect(item, SIGNAL(fullScreenChanged(bool)),
                   this, SLOT(setFullScreen(bool)));
    }

    d->shellSurfaceItem = item;
    if (item) {
        item->setParentItem(this);

        connect(item, &QWaylandQuickItem::surfaceChanged,
                this, &WaylandWindowItem::surfaceChanged);
        connect(item, &QWaylandQuickItem::widthChanged,
                this, &WaylandWindowItem::shellSurfaceItemWidthChanged);
        connect(item, &QWaylandQuickItem::heightChanged,
                this, &WaylandWindowItem::shellSurfaceItemHeightChanged);

        connect(item, SIGNAL(titleChanged(QString)),
                this, SLOT(setTitle(QString)));
        connect(item, SIGNAL(appIdChanged(QString)),
                this, SLOT(setAppId(QString)));

        connect(item, SIGNAL(activeChanged(bool)),
                this, SLOT(setActive(bool)));
        connect(item, SIGNAL(minimizedChanged(bool)),
                this, SLOT(setMinimized(bool)));
        connect(item, SIGNAL(maximizedChanged(bool)),
                this, SLOT(setMaximized(bool)));
        connect(item, SIGNAL(fullScreenChanged(bool)),
                this, SLOT(setFullScreen(bool)));
    }

    Q_EMIT shellSurfaceItemChanged();
}

QString WaylandWindowItem::title() const
{
    Q_D(const WaylandWindowItem);
    return d->title;
}

QString WaylandWindowItem::appId() const
{
    Q_D(const WaylandWindowItem);
    return d->appId;
}

bool WaylandWindowItem::isActive() const
{
    Q_D(const WaylandWindowItem);
    return d->active;
}

bool WaylandWindowItem::isMinimized() const
{
    Q_D(const WaylandWindowItem);
    return d->minimized;
}

bool WaylandWindowItem::isMaximized() const
{
    Q_D(const WaylandWindowItem);
    return d->maximized;
}

bool WaylandWindowItem::isFullScreen() const
{
    Q_D(const WaylandWindowItem);
    return d->fullScreen;
}

QPointF WaylandWindowItem::randomPosition() const
{
    if (!parentItem() || !shellSurfaceItem())
        return QPointF(0, 0);

    const int step = 24;
    static int px = step;
    static int py = 2 * step;
    int dx, dy;

    // Increment new coordinates by the step
    px += step;
    py += 2 * step;
    if (px > parentItem()->width() / 2)
        px = step;
    if (py > parentItem()->height() / 2)
        py = step;
    dx = px;
    dy = py;
    if (dx + width() > parentItem()->width()) {
        dx = parentItem()->width() - width();
        if (dx < 0)
            dx = 0;
        px = 0;
    }
    if (dy + height() > parentItem()->height()) {
        dy = parentItem()->height() - height();
        if (dy < 0)
            dy = 0;
        py = 0;
    }

    return QPointF(dx, dy);
}

void WaylandWindowItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(WaylandWindowItem);

    // If the modifier is pressed we initiate a move operation
    Qt::KeyboardModifier mod = Qt::MetaModifier;
    if (QGuiApplication::queryKeyboardModifiers().testFlag(mod) && event->buttons().testFlag(Qt::LeftButton)) {
        d->grabberState = WaylandWindowItemPrivate::MoveState;
        d->moveState.initialized = false;
        return;
    }

    QQuickItem::mousePressEvent(event);
}

void WaylandWindowItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(WaylandWindowItem);

    if (d->grabberState != WaylandWindowItemPrivate::DefaultState) {
        d->grabberState = WaylandWindowItemPrivate::DefaultState;
        return;
    }

    QQuickItem::mouseReleaseEvent(event);
}

void WaylandWindowItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(WaylandWindowItem);

    if (d->grabberState == WaylandWindowItemPrivate::MoveState) {
        if (!d->moveState.initialized) {
            d->moveState.initialOffset = mapFromItem(Q_NULLPTR, event->windowPos());
            d->moveState.initialized = true;
        }

        if (!parentItem())
            return;

        QPointF parentPos = parentItem()->mapFromItem(Q_NULLPTR, event->windowPos());
        QPointF pos = parentPos - d->moveState.initialOffset;
        setPosition(pos);

        return;
    }

    QQuickItem::mouseMoveEvent(event);
}

void WaylandWindowItem::lower()
{
    QQuickItem *parent = parentItem();
    Q_ASSERT(parent);
    QQuickItem *bottom = parent->childItems().first();
    if (this != bottom)
        stackBefore(bottom);
}

void WaylandWindowItem::raise()
{
    QQuickItem *parent = parentItem();
    Q_ASSERT(parent);
    QQuickItem *top = parent->childItems().last();
    if (this != top)
        stackAfter(top);
}

void WaylandWindowItem::shellSurfaceItemWidthChanged()
{
    Q_D(WaylandWindowItem);
    setWidth(d->shellSurfaceItem->width());
}

void WaylandWindowItem::shellSurfaceItemHeightChanged()
{
    Q_D(WaylandWindowItem);
    setHeight(d->shellSurfaceItem->height());
}

void WaylandWindowItem::setTitle(const QString &title)
{
    Q_D(WaylandWindowItem);

    if (d->title == title)
        return;

    d->title = title;
    Q_EMIT titleChanged();
}

void WaylandWindowItem::setAppId(const QString &appId)
{
    Q_D(WaylandWindowItem);

    if (d->appId == appId)
        return;

    d->appId = appId;
    Q_EMIT appIdChanged();
}

void WaylandWindowItem::setActive(bool active)
{
    Q_D(WaylandWindowItem);

    if (d->active == active)
        return;

    d->active = active;
    Q_EMIT activeChanged();
}

void WaylandWindowItem::setMinimized(bool minimized)
{
    Q_D(WaylandWindowItem);

    if (d->minimized == minimized)
        return;

    d->minimized = minimized;
    Q_EMIT minimizedChanged();
}

void WaylandWindowItem::setMaximized(bool maximized)
{
    Q_D(WaylandWindowItem);

    if (d->maximized == maximized)
        return;

    d->maximized = maximized;
    Q_EMIT maximizedChanged();
}

void WaylandWindowItem::setFullScreen(bool fullScreen)
{
    Q_D(WaylandWindowItem);

    if (d->fullScreen == fullScreen)
        return;

    d->fullScreen = fullScreen;
    Q_EMIT fullScreenChanged();
}

#include "moc_waylandwindowitem.cpp"
