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

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickShellSurfaceItem>

#include "clientwindow.h"
#include "clientwindow_p.h"
#include "clientwindowview.h"
#include "clientwindowview_p.h"
#include "serverlogging_p.h"
#include "windowmanager.h"
#include "windowmanager_p.h"
#include "extensions/quickxdgpopupitem.h"
#include "extensions/quickxdgsurfaceitem.h"

namespace GreenIsland {

namespace Server {

/*
 * ClientWindowViewPrivate
 */

ClientWindowViewPrivate::ClientWindowViewPrivate()
    : initialized(false)
    , output(Q_NULLPTR)
    , shellSurfaceItem(Q_NULLPTR)
    , savedProperties(new QQmlPropertyMap())
    , grabberState(DefaultState)
{
    savedProperties->insert(QStringLiteral("x"), qreal(0));
    savedProperties->insert(QStringLiteral("y"), qreal(0));
    savedProperties->insert(QStringLiteral("width"), qreal(0));
    savedProperties->insert(QStringLiteral("height"), qreal(0));
    savedProperties->insert(QStringLiteral("scale"), qreal(0));
}

ClientWindowViewPrivate::~ClientWindowViewPrivate()
{
    delete shellSurfaceItem;
    delete savedProperties;
}

void ClientWindowViewPrivate::setShellSurfaceItem(QWaylandQuickItem *item)
{
    Q_Q(ClientWindowView);

    if (shellSurfaceItem == item)
        return;

    if (shellSurfaceItem) {
        q->disconnect(shellSurfaceItem, &QWaylandQuickItem::widthChanged,
                      q, &ClientWindowView::shellSurfaceItemWidthChanged);
        q->disconnect(shellSurfaceItem, &QWaylandQuickItem::heightChanged,
                      q, &ClientWindowView::shellSurfaceItemHeightChanged);
    }

    shellSurfaceItem = item;
    if (item) {
        item->setParentItem(q);
        item->setX(0);
        item->setY(0);
        item->setZ(0);

        q->connect(item, &QWaylandQuickItem::widthChanged,
                   q, &ClientWindowView::shellSurfaceItemWidthChanged);
        q->connect(item, &QWaylandQuickItem::heightChanged,
                   q, &ClientWindowView::shellSurfaceItemHeightChanged);
    }

    Q_EMIT q->shellSurfaceItemChanged();
}

bool ClientWindowViewPrivate::mousePressEvent(QMouseEvent *event)
{
    // If the modifier is pressed we initiate a move operation
    Qt::KeyboardModifier mod = Qt::MetaModifier;
    if (QGuiApplication::queryKeyboardModifiers().testFlag(mod) && event->buttons().testFlag(Qt::LeftButton)) {
        grabberState = ClientWindowViewPrivate::MoveState;
        moveState.initialized = false;
        return true;
    }

    return false;
}

bool ClientWindowViewPrivate::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if (grabberState != ClientWindowViewPrivate::DefaultState) {
        grabberState = ClientWindowViewPrivate::DefaultState;
        return true;
    }

    return false;
}

bool ClientWindowViewPrivate::mouseMoveEvent(QMouseEvent *event)
{
    Q_Q(ClientWindowView);

    if (grabberState == ClientWindowViewPrivate::MoveState) {
        if (!moveState.initialized) {
            moveState.initialOffset = q->mapFromItem(Q_NULLPTR, event->windowPos());
            moveState.initialized = true;
        }

        if (!q->parentItem())
            return false;

        QQuickItem *moveItem = ClientWindowPrivate::get(window)->moveItem;
        QPointF parentPos = q->parentItem()->mapFromItem(Q_NULLPTR, event->windowPos());
        QPointF pos = parentPos - moveState.initialOffset;
        moveItem->setPosition(pos);

        return true;
    }

    return false;
}

/*
 * ClientWindowView
 */

ClientWindowView::ClientWindowView(QQuickItem *parent)
    : QQuickItem(*new ClientWindowViewPrivate(), parent)
{
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFiltersChildMouseEvents(true);

    Q_D(ClientWindowView);
    connect(d->savedProperties, &QQmlPropertyMap::valueChanged,
            this, &ClientWindowView::savedPropertiesChanged);
}

QWaylandQuickOutput *ClientWindowView::output() const
{
    Q_D(const ClientWindowView);
    return d->output;
}

QWaylandQuickItem *ClientWindowView::shellSurfaceItem() const
{
    Q_D(const ClientWindowView);
    return d->shellSurfaceItem;
}

QQmlPropertyMap *ClientWindowView::savedProperties() const
{
    Q_D(const ClientWindowView);
    return d->savedProperties;
}

void ClientWindowView::initialize(ClientWindow *window, QWaylandQuickOutput *output)
{
    Q_D(ClientWindowView);

    if (d->initialized) {
        qCWarning(gLcCore, "ClientWindowView is already initialized");
        return;
    }

    ClientWindowPrivate *dWindow = ClientWindowPrivate::get(window);

    // Create the shell surface item
    if (dWindow->interfaceName == QWaylandShellSurface::interfaceName()) {
        QWaylandShellSurface *shellSurface =
                QWaylandShellSurface::findIn(dWindow->surface);
        if (!shellSurface) {
            qCWarning(gLcCore, "Shell surface not found");
            return;
        }

        QWaylandQuickShellSurfaceItem *shellSurfaceItem =
                new QWaylandQuickShellSurfaceItem();
        shellSurfaceItem->setSurface(dWindow->surface);
        shellSurfaceItem->setShellSurface(shellSurface);

        d->setShellSurfaceItem(shellSurfaceItem);
        shellSurfaceItem->setFocusOnClick(true);
        shellSurfaceItem->setMoveItem(dWindow->moveItem);
    } else if (dWindow->interfaceName == XdgSurface::interfaceName()) {
        XdgSurface *shellSurface = XdgSurface::findIn(dWindow->surface);
        if (!shellSurface) {
            qCWarning(gLcCore, "Shell surface not found");
            return;
        }

        QuickXdgSurfaceItem *shellSurfaceItem =
                new QuickXdgSurfaceItem();
        shellSurfaceItem->setSurface(dWindow->surface);
        shellSurfaceItem->setShellSurface(shellSurface);

        d->setShellSurfaceItem(shellSurfaceItem);
        shellSurfaceItem->setFocusOnClick(true);
        shellSurfaceItem->setMoveItem(dWindow->moveItem);
    } else if (dWindow->interfaceName == XdgPopup::interfaceName()) {
        XdgPopup *shellSurface = XdgPopup::findIn(dWindow->surface);
        if (!shellSurface) {
            qCWarning(gLcCore, "Shell surface not found");
            return;
        }

        QuickXdgPopupItem *shellSurfaceItem =
                new QuickXdgPopupItem();
        shellSurfaceItem->setSurface(dWindow->surface);
        shellSurfaceItem->setShellSurface(shellSurface);

        d->setShellSurfaceItem(shellSurfaceItem);
        shellSurfaceItem->setFocusOnClick(true);
    } else {
        qCWarning(gLcCore, "Surface implements an unknown interface");
        return;
    }

    // Save
    d->window = window;
    d->output = output;
    dWindow->views.append(this);

    // Activate window when it has focus
    connect(shellSurfaceItem(), &QWaylandQuickItem::focusChanged, this, [this, d](bool focused) {
        ClientWindowPrivate::get(d->window)->setActive(focused);
    });

    // Finish initialization
    Q_EMIT outputChanged();
    Q_EMIT window->viewsChanged();
    d->initialized = true;
}

bool ClientWindowView::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    Q_D(ClientWindowView);

    // Make sure we will filter only events for the shell surface item
    if (item != d->shellSurfaceItem)
        return false;

    // Filter mouse events
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return d->mousePressEvent(static_cast<QMouseEvent *>(event));
    case QEvent::MouseButtonRelease:
        return d->mouseReleaseEvent(static_cast<QMouseEvent *>(event));
    case QEvent::MouseMove:
        return d->mouseMoveEvent(static_cast<QMouseEvent *>(event));
    default:
        break;
    }

    // Do not filter out events we don't care about
    return false;
}

void ClientWindowView::shellSurfaceItemWidthChanged()
{
    Q_D(ClientWindowView);
    setWidth(d->shellSurfaceItem->width());
}

void ClientWindowView::shellSurfaceItemHeightChanged()
{
    Q_D(ClientWindowView);
    setHeight(d->shellSurfaceItem->height());
}

} // namespace Server

} // namespace GreenIsland

#include "moc_clientwindowview.cpp"
