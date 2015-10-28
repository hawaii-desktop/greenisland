/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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
#include <QtGui/QScreen>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include "libinput/libinputhandler.h"
#include "libinput/libinputpointer.h"

#include <libinput.h>

namespace GreenIsland {

namespace Platform {

LibInputPointer::LibInputPointer(LibInputHandler *handler)
    : m_handler(handler)
    , m_buttons(Qt::NoButton)
{
}

void LibInputPointer::setPosition(const QPoint &pos)
{
    // Constrain position to the virtual desktop
    const QRect geometry = QGuiApplication::primaryScreen()->virtualGeometry();
    m_pt.setX(qBound(geometry.left(), pos.x(), geometry.right()));
    m_pt.setX(qBound(geometry.top(), pos.x(), geometry.bottom()));
}

void LibInputPointer::handleButton(libinput_event_pointer *e)
{
    Qt::MouseButton button = Qt::NoButton;
    switch (libinput_event_pointer_get_button(e)) {
    case 0x110: button = Qt::LeftButton; break;
    case 0x111: button = Qt::RightButton; break;
    case 0x112: button = Qt::MiddleButton; break;
    case 0x113: button = Qt::ExtraButton1; break;
    case 0x114: button = Qt::ExtraButton2; break;
    case 0x115: button = Qt::ExtraButton3; break;
    case 0x116: button = Qt::ExtraButton4; break;
    case 0x117: button = Qt::ExtraButton5; break;
    case 0x118: button = Qt::ExtraButton6; break;
    case 0x119: button = Qt::ExtraButton7; break;
    case 0x11a: button = Qt::ExtraButton8; break;
    case 0x11b: button = Qt::ExtraButton9; break;
    case 0x11c: button = Qt::ExtraButton10; break;
    case 0x11d: button = Qt::ExtraButton11; break;
    case 0x11e: button = Qt::ExtraButton12; break;
    case 0x11f: button = Qt::ExtraButton13; break;
    }

    if (libinput_event_pointer_get_button_state(e) == LIBINPUT_BUTTON_STATE_PRESSED)
        m_buttons |= button;
    else
        m_buttons &= ~button;

    LibInputMouseEvent event;
    event.pos = m_pt;
    event.buttons = m_buttons;
    event.modifiers = QGuiApplication::keyboardModifiers();
    event.wheelDelta = 0;
    event.wheelOrientation = Qt::Horizontal;
    if (libinput_event_pointer_get_button_state(e) == LIBINPUT_BUTTON_STATE_PRESSED)
        Q_EMIT m_handler->mousePressed(event);
    else
        Q_EMIT m_handler->mouseReleased(event);
}

void LibInputPointer::handleMotion(libinput_event_pointer *e)
{
    QPointF delta(libinput_event_pointer_get_dx(e),
                  libinput_event_pointer_get_dy(e));
    QPoint pos = m_pt + delta.toPoint();

    const QRect geometry = QGuiApplication::primaryScreen()->virtualGeometry();
    m_pt.setX(qBound(geometry.left(), pos.x(), geometry.right()));
    m_pt.setY(qBound(geometry.top(), pos.y(), geometry.bottom()));

    LibInputMouseEvent event;
    event.pos = m_pt;
    event.buttons = m_buttons;
    event.modifiers = QGuiApplication::keyboardModifiers();
    event.wheelDelta = 0;
    event.wheelOrientation = Qt::Horizontal;
    Q_EMIT m_handler->mouseMoved(event);
}

void LibInputPointer::handleAxis(libinput_event_pointer *e)
{
    LibInputMouseEvent event;
    event.pos = m_pt;
    event.buttons = m_buttons;
    event.modifiers = QGuiApplication::keyboardModifiers();

    if (libinput_event_pointer_has_axis(e, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
        const double d =
                libinput_event_pointer_get_axis_value(e,
                                                      LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
        event.wheelDelta = qRound(120 * d);
        event.wheelOrientation = Qt::Horizontal;
        Q_EMIT m_handler->mouseWheel(event);
    }

    if (libinput_event_pointer_has_axis(e, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
        const double d =
                libinput_event_pointer_get_axis_value(e,
                                                      LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
        event.wheelDelta = qRound(120 * d);
        event.wheelOrientation = Qt::Vertical;
        Q_EMIT m_handler->mouseWheel(event);
    }
}

} // namespace Platform

} // namespace GreenIsland