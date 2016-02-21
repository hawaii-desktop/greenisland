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
