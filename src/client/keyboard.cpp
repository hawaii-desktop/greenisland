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

#include "keyboard.h"
#include "keyboard_p.h"
#include "seat.h"
#include "surface_p.h"

#include <unistd.h>

namespace GreenIsland {

namespace Client {

/*
 * KeyboardPrivate
 */

KeyboardPrivate::KeyboardPrivate()
    : QtWayland::wl_keyboard()
    , seat(Q_NULLPTR)
    , seatVersion(0)
    , focusSurface(Q_NULLPTR)
    , repeatRate(0)
    , repeatDelay(0)
{
}

KeyboardPrivate::~KeyboardPrivate()
{
    if (seatVersion >= 3)
        release();
}

Keyboard *KeyboardPrivate::fromWlKeyboard(struct ::wl_keyboard *keyboard)
{
    QtWayland::wl_keyboard *wlKeyboard =
            static_cast<QtWayland::wl_keyboard *>(wl_keyboard_get_user_data(keyboard));
    return static_cast<KeyboardPrivate *>(wlKeyboard)->q_func();
}

void KeyboardPrivate::keyboard_keymap(uint32_t format, int32_t fd, uint32_t size)
{
    Q_Q(Keyboard);

    if (format != QtWayland::wl_keyboard::keymap_format_xkb_v1) {
        ::close(fd);
        return;
    }

    Q_EMIT q->keymapChanged(fd, size);
}

void KeyboardPrivate::keyboard_enter(uint32_t serial, struct ::wl_surface *surface, wl_array *keys)
{
    Q_UNUSED(keys);

    Q_Q(Keyboard);

    focusSurface = SurfacePrivate::fromWlSurface(surface);
    Q_EMIT q->enter(serial);
}

void KeyboardPrivate::keyboard_leave(uint32_t serial, struct ::wl_surface *surface)
{
    Q_UNUSED(surface);

    Q_Q(Keyboard);

    focusSurface = Q_NULLPTR;
    Q_EMIT q->leave(serial);
}

void KeyboardPrivate::keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(serial);

    Q_Q(Keyboard);

    if (state == QtWayland::wl_keyboard::key_state_pressed)
        Q_EMIT q->keyPressed(time, key);
    else
        Q_EMIT q->keyReleased(time, key);
}

void KeyboardPrivate::keyboard_modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    Q_UNUSED(serial);

    Q_Q(Keyboard);

    Q_EMIT q->modifiersChanged(mods_depressed, mods_latched, mods_locked, group);
}

void KeyboardPrivate::keyboard_repeat_info(int32_t rate, int32_t delay)
{
    Q_Q(Keyboard);

    if (qint32(repeatRate) != rate) {
        repeatRate = qMax(rate, 0);
        Q_EMIT q->repeatRateChanged();
    }

    if (qint32(repeatDelay) != delay) {
        repeatDelay = qMax(delay, 0);
        Q_EMIT q->repeatDelayChanged();
    }
}

/*
 * Keyboard
 */

Keyboard::Keyboard(Seat *seat)
    : QObject(*new KeyboardPrivate(), seat)
{
    d_func()->seat = seat;
    d_func()->seatVersion = seat->version();
}

Surface *Keyboard::focusSurface() const
{
    Q_D(const Keyboard);
    return d->focusSurface;
}

quint32 Keyboard::repeatRate() const
{
    Q_D(const Keyboard);
    return d->repeatRate;
}

quint32 Keyboard::repeatDelay() const
{
    Q_D(const Keyboard);
    return d->repeatDelay;
}

QByteArray Keyboard::interfaceName()
{
    return QByteArrayLiteral("wl_keyboard");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_keyboard.cpp"
