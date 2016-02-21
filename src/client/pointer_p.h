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

#ifndef GREENISLANDCLIENT_POINTER_P_H
#define GREENISLANDCLIENT_POINTER_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/Pointer>
#include <GreenIsland/Client/Surface>
#include <GreenIsland/client/private/qwayland-wayland.h>

#include <wayland-cursor.h>

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

class GREENISLANDCLIENT_EXPORT PointerPrivate
        : public QObjectPrivate
        , public QtWayland::wl_pointer
{
    Q_DECLARE_PUBLIC(Pointer)
public:
    PointerPrivate();
    ~PointerPrivate();

    Seat *seat;
    quint32 seatVersion;
    Surface *cursorSurface;
    Surface *focusSurface;
    quint32 enterSerial;

    static Pointer *fromWlPointer(struct ::wl_pointer *pointer);
    static PointerPrivate *get(Pointer *pointer) { return pointer->d_func(); }

protected:
    void pointer_enter(uint32_t serial, struct ::wl_surface *surface,
                       wl_fixed_t surface_x, wl_fixed_t surface_y) Q_DECL_OVERRIDE;
    void pointer_leave(uint32_t serial, struct ::wl_surface *surface) Q_DECL_OVERRIDE;
    void pointer_motion(uint32_t time, wl_fixed_t surface_x,
                        wl_fixed_t surface_y) Q_DECL_OVERRIDE;
    void pointer_button(uint32_t serial, uint32_t time, uint32_t button,
                        uint32_t state) Q_DECL_OVERRIDE;
    void pointer_axis(uint32_t time, uint32_t axis, wl_fixed_t value) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_POINTER_P_H
