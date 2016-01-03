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

#ifndef GREENISLANDCLIENT_SURFACE_P_H
#define GREENISLANDCLIENT_SURFACE_P_H

#include <QtCore/private/qobject_p.h>

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

class GREENISLANDCLIENT_EXPORT SurfacePrivate
        : public QObjectPrivate
        , public QtWayland::wl_surface
{
    Q_DECLARE_PUBLIC(Surface)
public:
    SurfacePrivate();

    bool frameCallbackInstalled;

    void handleFrameCallback();

    static void frameCallback(void *data, wl_callback *callback, uint32_t time);
    static const wl_callback_listener callbackListener;

    static Surface *fromWlSurface(struct ::wl_surface *surface);
    static SurfacePrivate *get(Surface *surface) { return surface->d_func(); }

protected:
    void surface_enter(wl_output *output) Q_DECL_OVERRIDE;
    void surface_leave(wl_output *output) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SURFACE_P_H
