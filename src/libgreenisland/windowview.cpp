/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "abstractcompositor.h"
#include "inputdevice.h"
#include "quicksurface.h"

#include "clientwindow.h"
#include "compositor.h"
#include "compositorsettings.h"
#include "windowview.h"

namespace GreenIsland {

WindowView::WindowView(QuickSurface *surface, QQuickItem *parent)
    : SurfaceItem(surface, parent)
    , m_pos(0, 0)
{
}

QPointF WindowView::localPosition() const
{
    return m_pos;
}

void WindowView::setLocalPosition(const QPointF &pt)
{
    if (m_pos == pt)
        return;

    m_pos = pt;
    Q_EMIT localPositionChanged();
}

qreal WindowView::localX() const
{
    return localPosition().x();
}

qreal WindowView::localY() const
{
    return localPosition().y();
}

void WindowView::mousePressEvent(QMouseEvent *event)
{
    // Must be first to correctly set the pointer focus
    SurfaceItem::mousePressEvent(event);

    // Emit a mouse pressed signal, ClientWindow will connect
    // and activate the window
    Q_EMIT mousePressed();

    // If the modifier is pressed we initiate a move operation
    Qt::KeyboardModifier mod = static_cast<Compositor *>(surface()->compositor())->settings()->windowActionKey();
    if (QGuiApplication::queryKeyboardModifiers() & mod)
        startMove();
}

void WindowView::mouseReleaseEvent(QMouseEvent *event)
{
    // Stop the move operation nevertheless
    stopMove();

    SurfaceItem::mouseReleaseEvent(event);
}

void WindowView::mouseMoveEvent(QMouseEvent *event)
{
    // Stop the move operation if the modifier is not pressed anymore
    Qt::KeyboardModifier mod = static_cast<Compositor *>(surface()->compositor())->settings()->windowActionKey();
    if (!(QGuiApplication::queryKeyboardModifiers() & mod))
        stopMove();

    SurfaceItem::mouseMoveEvent(event);
}

void WindowView::takeFocus(InputDevice *device)
{
    SurfaceItem::takeFocus(device);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    forceActiveFocus();
#endif
}

void WindowView::startMove()
{
    SurfaceOperation op(ClientWindow::Move);
    surface()->sendInterfaceOp(op);
}

void WindowView::stopMove()
{
    SurfaceOperation op(ClientWindow::StopMove);
    surface()->sendInterfaceOp(op);
}

}

#include "moc_windowview.cpp"
