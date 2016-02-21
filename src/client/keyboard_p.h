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
