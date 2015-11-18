/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>

#include "quickxdgpopupitem.h"
#include "quickxdgpopupitem_p.h"
#include "xdgshell_p.h"

namespace GreenIsland {

namespace Server {

QuickXdgPopupItem::QuickXdgPopupItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QuickXdgPopupItemPrivate(), parent)
{
}

QuickXdgPopupItem::QuickXdgPopupItem(QuickXdgPopupItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
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

void QuickXdgPopupItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(QuickXdgPopupItem);

    if (shellSurface()) {
        XdgPopupPrivate *popupPrivate =
                XdgPopupPrivate::get(shellSurface());
        XdgShellPrivate *shellPrivate =
                XdgShellPrivate::get(popupPrivate->getShell());

        if (shellPrivate->isPopupInitialUp()) {
            shellSurface()->sendPopupDone();
            shellPrivate->removePopup(shellSurface());
            //shellPrivate->m_popups.clear();
        }
    }

    QWaylandQuickItem::mousePressEvent(event);
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

void QuickXdgPopupItem::adjustOffsetForNextFrame(const QPointF &offset)
{
    Q_D(QuickXdgPopupItem);
    setPosition(position() + offset);
}

} // namespace Server

} // namespace GreenIsland

#include "moc_quickxdgpopupitem.cpp"
