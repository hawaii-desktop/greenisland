/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include "libinput/libinputhandler.h"
#include "libinput/libinputpointer.h"

#include <libinput.h>

#include <QtGui/private/qhighdpiscaling_p.h>

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
    QScreen *const primaryScreen = QGuiApplication::primaryScreen();
    const QRect geometry = QHighDpi::toNativePixels(primaryScreen->virtualGeometry(), primaryScreen);
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    const bool pressed = libinput_event_pointer_get_button_state(e) == LIBINPUT_BUTTON_STATE_PRESSED;
    m_buttons.setFlag(button, pressed);
#else
    if (libinput_event_pointer_get_button_state(e) == LIBINPUT_BUTTON_STATE_PRESSED)
        m_buttons |= button;
    else
        m_buttons &= ~button;
#endif

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
    processMotion(pos);
}

void LibInputPointer::handleAbsoluteMotion(libinput_event_pointer *e)
{
    QScreen *const primaryScreen = QGuiApplication::primaryScreen();
    const QRect geometry = QHighDpi::toNativePixels(primaryScreen->virtualGeometry(), primaryScreen);
    QPointF abs(libinput_event_pointer_get_absolute_x_transformed(e, geometry.size().width()),
                  libinput_event_pointer_get_absolute_y_transformed(e, geometry.size().height()));
    processMotion(abs.toPoint());
}

void LibInputPointer::handleAxis(libinput_event_pointer *e)
{
    LibInputMouseEvent event;
    event.pos = m_pt;
    event.buttons = m_buttons;
    event.modifiers = QGuiApplication::keyboardModifiers();

    // TODO: Make sensitivity configurable instead of fixed 10
    const double sensitivity = qBound<double>(1, 10, 100);

    if (libinput_event_pointer_has_axis(e, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
        const double d =
                libinput_event_pointer_get_axis_value(e,
                                                      LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
        event.wheelDelta = qRound(-d * sensitivity);
        event.wheelOrientation = Qt::Horizontal;
        Q_EMIT m_handler->mouseWheel(event);
    }

    if (libinput_event_pointer_has_axis(e, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
        const double d =
                libinput_event_pointer_get_axis_value(e,
                                                      LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
        event.wheelDelta = qRound(-d * sensitivity);
        event.wheelOrientation = Qt::Vertical;
        Q_EMIT m_handler->mouseWheel(event);
    }
}

void LibInputPointer::processMotion(const QPoint &pos)
{
    QScreen *const primaryScreen = QGuiApplication::primaryScreen();
    const QRect geometry = QHighDpi::toNativePixels(primaryScreen->virtualGeometry(), primaryScreen);
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

} // namespace Platform

} // namespace GreenIsland
