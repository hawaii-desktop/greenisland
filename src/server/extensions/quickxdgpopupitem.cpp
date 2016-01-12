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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>

#include "quickxdgpopupitem.h"
#include "quickxdgpopupitem_p.h"
#include "xdgshell_p.h"

namespace GreenIsland {

namespace Server {

/*
 * QuickXdgPopupItemPrivate
 */

QuickXdgPopupItemPrivate::QuickXdgPopupItemPrivate()
    : QWaylandQuickItemPrivate()
    , window(Q_NULLPTR)
    , shellSurface(Q_NULLPTR)
{
}

QuickXdgPopupItemPrivate::~QuickXdgPopupItemPrivate()
{
    Q_Q(QuickXdgPopupItem);

    if (window)
        window->removeEventFilter(q);
}

bool QuickXdgPopupItemPrivate::processMousePressEvent(QMouseEvent *event)
{
    Q_Q(QuickXdgPopupItem);

    if (!shellSurface || !window)
        return false;

    XdgPopupPrivate *popupPrivate = XdgPopupPrivate::get(shellSurface);
    XdgShellPrivate *shellPrivate = XdgShellPrivate::get(popupPrivate->getShell());

    QPointF scenePos = window->contentItem()->mapToItem(q, event->windowPos());

    // Close popup menu if user clicked outside otherwise do not
    // filter the event so it will be received by the popup
    if (!q->contains(scenePos)) {
        shellSurface->sendPopupDone();
        shellPrivate->removePopup(shellSurface);
        return true;
    }

    return false;
}

/*
 * QuickXdgPopupItem
 */

QuickXdgPopupItem::QuickXdgPopupItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QuickXdgPopupItemPrivate(), parent)
{
    connect(this, &QuickXdgPopupItem::windowChanged,
            this, &QuickXdgPopupItem::handleWindowChanged);
}

QuickXdgPopupItem::QuickXdgPopupItem(QuickXdgPopupItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
    connect(this, &QuickXdgPopupItem::windowChanged,
            this, &QuickXdgPopupItem::handleWindowChanged);
}

XdgPopup *QuickXdgPopupItem::shellSurface() const
{
    Q_D(const QuickXdgPopupItem);
    return d->shellSurface;
}

void QuickXdgPopupItem::setShellSurface(XdgPopup *shellSurface)
{
    Q_D(QuickXdgPopupItem);

    if (shellSurface == d->shellSurface)
        return;

    d->shellSurface = shellSurface;
    Q_EMIT shellSurfaceChanged();
}

bool QuickXdgPopupItem::eventFilter(QObject *object, QEvent *event)
{
    Q_D(QuickXdgPopupItem);

    if (object == this)
        return QObject::eventFilter(object, event);

    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return d->processMousePressEvent(static_cast<QMouseEvent *>(event));
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

void QuickXdgPopupItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QuickXdgPopupItem);

    if (shellSurface()) {
        XdgPopupPrivate *popupPrivate =
                XdgPopupPrivate::get(shellSurface());
        XdgShellPrivate *shellPrivate =
                XdgShellPrivate::get(popupPrivate->getShell());

        shellPrivate->setPopupInitialUp(false);
    }

    QWaylandQuickItem::mouseReleaseEvent(event);
}

void QuickXdgPopupItem::surfaceChangedEvent(QWaylandSurface *newSurface,
                                            QWaylandSurface *oldSurface)
{
    if (oldSurface)
        disconnect(oldSurface, &QWaylandSurface::offsetForNextFrame,
                   this, &QuickXdgPopupItem::adjustOffsetForNextFrame);
    if (newSurface)
        connect(newSurface, &QWaylandSurface::offsetForNextFrame,
                this, &QuickXdgPopupItem::adjustOffsetForNextFrame);
}

void QuickXdgPopupItem::componentComplete()
{
    Q_D(QuickXdgPopupItem);

    if (!d->shellSurface)
        setShellSurface(new XdgPopup());

    QWaylandQuickItem::componentComplete();
}

void QuickXdgPopupItem::handleWindowChanged(QQuickWindow *window)
{
    Q_D(QuickXdgPopupItem);

    // Remove event filter from the previous window
    if (d->window)
        d->window->removeEventFilter(this);

    // Install event filter on the new window
    d->window = window;
    if (d->window)
        d->window->installEventFilter(this);
}

void QuickXdgPopupItem::adjustOffsetForNextFrame(const QPointF &offset)
{
    Q_D(QuickXdgPopupItem);
    setPosition(position() + offset);
}

} // namespace Server

} // namespace GreenIsland

#include "moc_quickxdgpopupitem.cpp"
