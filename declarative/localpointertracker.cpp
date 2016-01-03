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

#include <QtQuick/private/qquickitem_p.h>

#include "globalpointertracker.h"
#include "localpointertracker.h"

class LocalPointerTrackerPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(LocalPointerTracker)
public:
    LocalPointerTrackerPrivate()
        : globalPointerTracker(Q_NULLPTR)
    {}

    void handlePos(const QPointF &localPos)
    {
        Q_Q(LocalPointerTracker);

        bool xChanged = localPos.x() != mousePos.x();
        bool yChanged = localPos.y() != mousePos.y();

        if (xChanged || yChanged) {
            mousePos = localPos;

            if (xChanged)
                Q_EMIT q->mouseXChanged();
            if (yChanged)
                Q_EMIT q->mouseYChanged();
        }

        if (globalPointerTracker) {
            QPointF globalPos = q->window()->mapToGlobal(localPos.toPoint());
            globalPointerTracker->setMouseX(globalPos.x());
            globalPointerTracker->setMouseY(globalPos.y());
        }
    }

    GlobalPointerTracker *globalPointerTracker;
    QPointF mousePos;
};

LocalPointerTracker::LocalPointerTracker(QQuickItem *parent)
    : QQuickItem(*new LocalPointerTrackerPrivate(), parent)
{
    setFiltersChildMouseEvents(true);
    setAcceptHoverEvents(true);
}

qreal LocalPointerTracker::mouseX() const
{
    Q_D(const LocalPointerTracker);
    return d->mousePos.x();
}

qreal LocalPointerTracker::mouseY() const
{
    Q_D(const LocalPointerTracker);
    return d->mousePos.y();
}

GlobalPointerTracker *LocalPointerTracker::globalTracker() const
{
    Q_D(const LocalPointerTracker);
    return d->globalPointerTracker;
}

void LocalPointerTracker::setGlobalTracker(GlobalPointerTracker *tracker)
{
    Q_D(LocalPointerTracker);

    if (d->globalPointerTracker == tracker)
        return;

    d->globalPointerTracker = tracker;
    Q_EMIT globalTrackerChanged();
}

bool LocalPointerTracker::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    Q_D(LocalPointerTracker);

    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        d->handlePos(mapFromItem(item, mouseEvent->localPos()));
    } else if (event->type() == QEvent::HoverMove) {
        QHoverEvent *hoverEvent = static_cast<QHoverEvent *>(event);
        d->handlePos(mapFromItem(item, hoverEvent->posF()));
    }

    return false;
}

void LocalPointerTracker::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(LocalPointerTracker);

    QQuickItem::mouseMoveEvent(event);

    d->handlePos(event->localPos());
}

void LocalPointerTracker::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(LocalPointerTracker);

    QQuickItem::hoverMoveEvent(event);

    d->handlePos(event->posF());
}

#include "moc_localpointertracker.cpp"
