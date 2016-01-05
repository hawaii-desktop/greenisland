/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLANDCLIENT_KEYBOARD_P_H
#define GREENISLANDCLIENT_KEYBOARD_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/Keyboard>
#include <GreenIsland/Client/Surface>
#include <GreenIsland/client/private/qwayland-wayland.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT KeyboardPrivate
        : public QObjectPrivate
        , public QtWayland::wl_keyboard
{
    Q_DECLARE_PUBLIC(Keyboard)
public:
    KeyboardPrivate();
    ~KeyboardPrivate();

    Seat *seat;
    quint32 seatVersion;
    Surface *focusSurface;
    quint32 repeatRate;
    quint32 repeatDelay;

    static Keyboard *fromWlKeyboard(struct ::wl_keyboard *keyboard);
    static KeyboardPrivate *get(Keyboard *keyboard) { return keyboard->d_func(); }

protected:
    void keyboard_keymap(uint32_t format, int32_t fd,
                         uint32_t size) Q_DECL_OVERRIDE;
    void keyboard_enter(uint32_t serial, struct ::wl_surface *surface,
                        wl_array *keys) Q_DECL_OVERRIDE;
    void keyboard_leave(uint32_t serial, struct ::wl_surface *surface) Q_DECL_OVERRIDE;
    void keyboard_key(uint32_t serial, uint32_t time, uint32_t key,
                      uint32_t state) Q_DECL_OVERRIDE;
    void keyboard_modifiers(uint32_t serial, uint32_t mods_depressed,
                            uint32_t mods_latched, uint32_t mods_locked,
                            uint32_t group) Q_DECL_OVERRIDE;
    void keyboard_repeat_info(int32_t rate, int32_t delay) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_KEYBOARD_P_H
