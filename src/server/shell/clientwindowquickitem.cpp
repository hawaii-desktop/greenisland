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

#include <QtGui/QGuiApplication>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandWlShell>
#include <GreenIsland/QtWaylandCompositor/QWaylandXdgShell>

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "clientwindowquickitem.h"
#include "clientwindowquickitem_p.h"
#include "serverlogging_p.h"

namespace GreenIsland {

namespace Server {

/*
 * ClientWindowQuickItemPrivate
 */

ClientWindowQuickItemPrivate::ClientWindowQuickItemPrivate()
    : QWaylandQuickShellSurfaceItemPrivate()
    , savedProperties(new QQmlPropertyMap())
    , isModifierHeld(false)
{
    savedProperties->insert(QStringLiteral("x"), qreal(0));
    savedProperties->insert(QStringLiteral("y"), qreal(0));
    savedProperties->insert(QStringLiteral("width"), qreal(0));
    savedProperties->insert(QStringLiteral("height"), qreal(0));
    savedProperties->insert(QStringLiteral("scale"), qreal(0));
}

ClientWindowQuickItemPrivate::~ClientWindowQuickItemPrivate()
{
    delete savedProperties;
}

/*
 * ClientWindowQuickItem
 */

ClientWindowQuickItem::ClientWindowQuickItem(QQuickItem *parent)
    : QWaylandQuickShellSurfaceItem(*new ClientWindowQuickItemPrivate(), parent)
{
    Q_D(ClientWindowQuickItem);
    connect(d->savedProperties, &QQmlPropertyMap::valueChanged,
            this, &ClientWindowQuickItem::savedPropertiesChanged);
}

QQmlPropertyMap *ClientWindowQuickItem::savedProperties() const
{
    Q_D(const ClientWindowQuickItem);
    return d->savedProperties;
}

void ClientWindowQuickItem::keyPressEvent(QKeyEvent *event)
{
    Q_D(ClientWindowQuickItem);

    d->isModifierHeld = event->key() == Qt::Key_Meta ||
            event->key() == Qt::Key_Super_L ||
            event->key() == Qt::Key_Super_R;

    QWaylandQuickShellSurfaceItem::keyPressEvent(event);
}

void ClientWindowQuickItem::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(ClientWindowQuickItem);

    if (event->key() == Qt::Key_Meta || event->key() == Qt::Key_Super_L ||
            event->key() == Qt::Key_Super_R)
        d->isModifierHeld = false;

    QWaylandQuickShellSurfaceItem::keyReleaseEvent(event);
}

void ClientWindowQuickItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(ClientWindowQuickItem);

    // Let mouse press go through anyway, if focus on click is enabled this
    // will give focus to the window before the use can drag it
    QWaylandQuickShellSurfaceItem::mousePressEvent(event);

    // If the modifier is pressed we initiate a move operation
    if (d->isModifierHeld && event->buttons().testFlag(Qt::LeftButton)) {
        QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface *>(shellSurface());
        if (wlShellSurface) {
            Q_EMIT wlShellSurface->startMove(compositor()->defaultSeat());
            return;
        }

        QWaylandXdgSurface *xdgSurface = qobject_cast<QWaylandXdgSurface *>(shellSurface());
        if (xdgSurface) {
            Q_EMIT xdgSurface->startMove(compositor()->defaultSeat());
            return;
        }
    }
}

} // namespace Server

} // namespace GreenIsland

#include "moc_clientwindowquickitem.cpp"
