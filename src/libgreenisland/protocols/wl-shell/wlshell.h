/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef WLSHELL_H
#define WLSHELL_H

#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>
#include <QtCompositor/QWaylandGlobalInterface>
#include <QtCompositor/private/qwayland-server-wayland.h>

Q_DECLARE_LOGGING_CATEGORY(WLSHELL_PROTOCOL)

namespace QtWayland {
class InputDevice;
}

namespace GreenIsland {

class WlShellSurface;
class WlShellSurfacePopupGrabber;

class WlShell : public QWaylandGlobalInterface, public QtWaylandServer::wl_shell
{
public:
    explicit WlShell();

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;

private:
    QHash<QtWayland::InputDevice *, WlShellSurfacePopupGrabber *> m_popupGrabbers;

    WlShellSurfacePopupGrabber *popupGrabberForDevice(QtWayland::InputDevice *device);

    void shell_get_shell_surface(Resource *resource, uint32_t id,
                                 wl_resource *surfaceResource) Q_DECL_OVERRIDE;

    friend class WlShellSurface;
};

}

#endif // WLSHELL_H
