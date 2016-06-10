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

void ClientWindowQuickItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(ClientWindowQuickItem);

    // If the modifier is pressed we initiate a move operation
    Qt::KeyboardModifier mod = Qt::MetaModifier;
    if (QGuiApplication::queryKeyboardModifiers().testFlag(mod) && event->buttons().testFlag(Qt::LeftButton)) {
        QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface *>(shellSurface());
        if (wlShellSurface) {
            Q_EMIT wlShellSurface->startMove(compositor()->defaultInputDevice());
            return;
        }

        QWaylandXdgSurface *xdgSurface = qobject_cast<QWaylandXdgSurface *>(shellSurface());
        if (xdgSurface) {
            Q_EMIT xdgSurface->startMove(compositor()->defaultInputDevice());
            return;
        }
    }

    // Do not filter out events we don't care about
    QWaylandQuickShellSurfaceItem::mousePressEvent(event);
}

} // namespace Server

} // namespace GreenIsland

#include "moc_clientwindowquickitem.cpp"
