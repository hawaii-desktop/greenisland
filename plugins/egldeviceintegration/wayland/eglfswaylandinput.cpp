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

#include <GreenIsland/Platform/EglFSXkb>

#include "eglfswaylandinput.h"
#include "eglfswaylandlogging.h"
#include "eglfswaylandwindow.h"

#include <sys/mman.h>
#include <unistd.h>

namespace GreenIsland {

namespace Platform {

EglFSWaylandInput::EglFSWaylandInput(Client::Seat *seat, QObject *parent)
    : QObject(parent)
    , m_seat(seat)
    , m_modifiers(Qt::NoModifier)
    , m_nativeModifiers(0)
    , m_mouseButtons(Qt::NoButton)
    , m_mouseLocalPos(0, 0)
    , m_mouseGlobalPos(0, 0)
    , m_touchDevice(Q_NULLPTR)
    , m_xkbContext(Q_NULLPTR)
    , m_xkbKeymap(Q_NULLPTR)
    , m_xkbState(Q_NULLPTR)
    , m_repeatKey(0)
    , m_repeatCode(0)
    , m_repeatTime(0)
{
    connect(&m_repeatTimer, &QTimer::timeout, this, &EglFSWaylandInput::repeatKey);
}

void EglFSWaylandInput::setTouchDevice(QTouchDevice *td)
{
    m_touchDevice = td;
}

void EglFSWaylandInput::keymapChanged(int fd, quint32 size)
{
    // Load keymap
    char *map = static_cast<char *>(::mmap(Q_NULLPTR, size, PROT_READ, MAP_SHARED, fd, 0));
    if (map == MAP_FAILED) {
        ::close(fd);
        return;
    }

    // Release the old keymap
    releaseKeymap();

    m_xkbContext = xkb_context_new(xkb_context_flags(0));
    m_xkbKeymap = xkb_map_new_from_string(m_xkbContext, map, XKB_KEYMAP_FORMAT_TEXT_V1,
                                          XKB_KEYMAP_COMPILE_NO_FLAGS);

    ::munmap(map, size);
    ::close(fd);

    m_xkbState = xkb_state_new(m_xkbKeymap);
}

void EglFSWaylandInput::keyPressed(quint32 time, quint32 key)
{
    processKeyEvent(QEvent::KeyPress, time, key);
}

void EglFSWaylandInput::keyReleased(quint32 time, quint32 key)
{
    processKeyEvent(QEvent::KeyRelease, time, key);
}

void EglFSWaylandInput::keyboardModifiersChanged(quint32 depressed,
                                                 quint32 latched,
                                                 quint32 locked,
                                                 quint32 group)
{
    if (m_xkbState)
        xkb_state_update_mask(m_xkbState, depressed, latched, locked, 0, 0, group);

    m_modifiers = EglFSXkb::modifiers(m_xkbState);
    m_nativeModifiers = depressed | latched | locked;
}

void EglFSWaylandInput::pointerEnter(quint32 serial, const QPointF &pos)
{
    Q_UNUSED(serial);

    m_mouseLocalPos = pos;

    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->pointer()->focusSurface());
    if (window)
        m_mouseGlobalPos = window->mapToGlobal(pos.toPoint());
    else
        m_mouseGlobalPos = pos;
}

void EglFSWaylandInput::pointerLeave(quint32 serial)
{
    Q_UNUSED(serial);

    m_mouseButtons = Qt::NoButton;
}

void EglFSWaylandInput::pointerMotion(quint32 time, const QPointF &pos)
{
    m_mouseLocalPos = pos;
    m_mouseGlobalPos = pos;

    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->pointer()->focusSurface());
    if (window)
        m_mouseGlobalPos = window->mapToGlobal(pos.toPoint());

    QWindowSystemInterface::handleMouseEvent(window ? window->window() : Q_NULLPTR,
                                             time, m_mouseLocalPos,
                                             m_mouseGlobalPos, m_mouseButtons);
}

void EglFSWaylandInput::pointerButtonPressed(quint32 serial, quint32 time, const Qt::MouseButton &button)
{
    Q_UNUSED(serial);

    m_mouseButtons |= button;

    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->pointer()->focusSurface());

    QWindowSystemInterface::handleMouseEvent(window ? window->window() : Q_NULLPTR,
                                             time, m_mouseLocalPos,
                                             m_mouseGlobalPos, m_mouseButtons);
}

void EglFSWaylandInput::pointerButtonReleased(quint32 serial, quint32 time, const Qt::MouseButton &button)
{
    Q_UNUSED(serial);

    m_mouseButtons &= ~button;

    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->pointer()->focusSurface());

    QWindowSystemInterface::handleMouseEvent(window ? window->window() : Q_NULLPTR,
                                             time, m_mouseLocalPos,
                                             m_mouseGlobalPos, m_mouseButtons);
}

void EglFSWaylandInput::pointerAxisChanged(quint32 time, const Qt::Orientation &orientation, qreal value)
{
    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->pointer()->focusSurface());

    QPoint angleDelta(0, 0);
    if (orientation == Qt::Horizontal)
        angleDelta.setX(value * -12);
    else
        angleDelta.setY(value * -12);
    QWindowSystemInterface::handleWheelEvent(window ? window->window() : Q_NULLPTR,
                                             time, m_mouseLocalPos, m_mouseGlobalPos,
                                             QPoint(), angleDelta);
}

void EglFSWaylandInput::touchSequenceStarted(Client::TouchPoint *p)
{
    QWindowSystemInterface::TouchPoint tp;
    tp.id = p->id();
    tp.pressure = p->isDown() ? 0.0 : 1.0;
    tp.rawPositions = p->positions();
    m_touchPoints.append(tp);
    QWindowSystemInterface::handleTouchEvent(Q_NULLPTR, m_touchDevice, m_touchPoints);
}

void EglFSWaylandInput::touchSequenceFinished()
{
    m_touchPoints.clear();
    QWindowSystemInterface::handleTouchEvent(Q_NULLPTR, m_touchDevice, m_touchPoints);
}

void EglFSWaylandInput::touchSequenceCanceled()
{
QWindowSystemInterface::handleTouchCancelEvent(Q_NULLPTR, m_touchDevice);
}

void EglFSWaylandInput::touchPointAdded(Client::TouchPoint *p)
{
    QWindowSystemInterface::TouchPoint tp;
    tp.id = p->id();
    tp.pressure = p->isDown() ? 0.0 : 1.0;
    tp.rawPositions = p->positions();
    m_touchPoints.append(tp);
    QWindowSystemInterface::handleTouchEvent(Q_NULLPTR, m_touchDevice, m_touchPoints);
}

void EglFSWaylandInput::touchPointRemoved(Client::TouchPoint *p)
{
    for (int i = 0; i < m_touchPoints.count(); i++) {
        if (m_touchPoints.at(i).id == p->id()) {
            m_touchPoints.removeAt(i);
            break;
        }
    }
    QWindowSystemInterface::handleTouchEvent(Q_NULLPTR, m_touchDevice, m_touchPoints);
}

void EglFSWaylandInput::touchPointMoved(Client::TouchPoint *p)
{
    for (int i = 0; i < m_touchPoints.count(); i++) {
        if (m_touchPoints.at(i).id == p->id()) {
            m_touchPoints.removeAt(i);

            QWindowSystemInterface::TouchPoint tp = m_touchPoints.at(i);
            tp.pressure = p->isDown() ? 0.0 : 1.0;
            tp.rawPositions = p->positions();
            m_touchPoints.append(tp);

            break;
        }
    }
    QWindowSystemInterface::handleTouchEvent(Q_NULLPTR, m_touchDevice, m_touchPoints);
}

bool EglFSWaylandInput::createDefaultKeymap()
{
    if (m_xkbContext && m_xkbKeymap && m_xkbState)
        return true;

    xkb_rule_names names;
    names.rules = ::strdup("evdev");
    names.model = ::strdup("pc105");
    names.layout = ::strdup("us");
    names.variant = ::strdup("");
    names.options = ::strdup("");

    m_xkbContext = xkb_context_new(xkb_context_flags(0));
    if (m_xkbContext) {
        m_xkbKeymap = xkb_map_new_from_names(m_xkbContext, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (m_xkbKeymap)
            m_xkbState = xkb_state_new(m_xkbKeymap);
    }

    if (!m_xkbContext || !m_xkbKeymap || !m_xkbState) {
        qCWarning(gLcEglFSWayland) << "Failed to load default keymap, keyboard input disabled";
        return false;
    }

    return true;
}

void EglFSWaylandInput::releaseKeymap()
{
    if (m_xkbState)
        xkb_state_unref(m_xkbState);
    if (m_xkbKeymap)
        xkb_map_unref(m_xkbKeymap);
    if (m_xkbContext)
        xkb_context_unref(m_xkbContext);
}

void EglFSWaylandInput::processKeyEvent(QEvent::Type type, quint32 time, quint32 key)
{
    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->keyboard()->focusSurface());

    if (!createDefaultKeymap())
        return;

    quint32 code = key + 8;
    QString text;

    const xkb_keysym_t sym = xkb_state_key_get_one_sym(m_xkbState, code);
    quint32 utf32 = xkb_keysym_to_utf32(sym);
    if (utf32)
        text = QString::fromUcs4(&utf32, 1);

    int qtKey = EglFSXkb::keysymToQtKey(sym, m_modifiers, text);

    QWindowSystemInterface::handleExtendedKeyEvent(window ? window->window() : Q_NULLPTR,
                                                   time, type, qtKey,
                                                   m_modifiers, code, sym,
                                                   m_nativeModifiers, text);

    if (type == QEvent::KeyPress && xkb_keymap_key_repeats(m_xkbKeymap, code)) {
        m_repeatKey = qtKey;
        m_repeatCode = code;
        m_repeatTime = time;
        m_repeatText = text;
        m_repeatSym = sym;
        m_repeatTimer.setInterval(m_seat->keyboard()->repeatRate());
        m_repeatTimer.start();
    } else if (m_repeatCode == code) {
        m_repeatTimer.stop();
    }
}

void EglFSWaylandInput::repeatKey()
{
    m_repeatTimer.setInterval(m_seat->keyboard()->repeatDelay());

    EglFSWaylandWindow *window = EglFSWaylandWindow::fromSurface(
                m_seat->keyboard()->focusSurface());

    QWindowSystemInterface::handleExtendedKeyEvent(window ? window->window() : Q_NULLPTR,
                                                   m_repeatTime, QEvent::KeyRelease,
                                                   m_repeatKey, m_modifiers,
                                                   m_repeatCode, m_repeatSym,
                                                   m_nativeModifiers, m_repeatText, true);
    QWindowSystemInterface::handleExtendedKeyEvent(window ? window->window() : Q_NULLPTR,
                                                   m_repeatTime, QEvent::KeyPress,
                                                   m_repeatKey, m_modifiers,
                                                   m_repeatCode, m_repeatSym,
                                                   m_nativeModifiers, m_repeatText, true);
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_eglfswaylandinput.cpp"
