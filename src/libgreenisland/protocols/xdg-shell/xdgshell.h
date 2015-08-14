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

#ifndef XDGSHELL_H
#define XDGSHELL_H

#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>

#include "globalinterface.h"

#include "qwayland-server-xdg-shell.h"

Q_DECLARE_LOGGING_CATEGORY(XDGSHELL_PROTOCOL)
Q_DECLARE_LOGGING_CATEGORY(XDGSHELL_TRACE)

namespace GreenIsland {
class WlInputDevice;
}

namespace GreenIsland {

class Compositor;
class XdgSurface;
class XdgPopup;
class XdgPopupGrabber;

class XdgShellGlobal : public QObject, public GlobalInterface
{
    Q_OBJECT
public:
    explicit XdgShellGlobal(Compositor *compositor);

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;

    inline Compositor *compositor() const { return m_compositor; }

private:
    Compositor *m_compositor;
};

class XdgShell : public QObject, public QtWaylandServer::xdg_shell
{
public:
    XdgShell(wl_client *client, uint32_t name, uint32_t version, QObject *parent);
    ~XdgShell();

    void pingSurface(XdgSurface *surface);

protected:
    void shell_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void shell_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void shell_use_unstable_version(Resource *resource, int32_t version) Q_DECL_OVERRIDE;
    void shell_get_xdg_surface(Resource *resource, uint32_t id,
                               wl_resource *surfaceResource)  Q_DECL_OVERRIDE;
    void shell_get_xdg_popup(Resource *resource, uint32_t id, wl_resource *surfaceResource,
                             wl_resource *parentResource, wl_resource *seatResource,
                             uint32_t serial, int32_t x, int32_t y)  Q_DECL_OVERRIDE;
    void shell_pong(Resource *resource, uint32_t serial)  Q_DECL_OVERRIDE;

private:
    QMap<uint32_t, XdgSurface *> m_pings;
    QHash<GreenIsland::WlInputDevice *, XdgPopupGrabber *> m_popupGrabbers;

    XdgPopupGrabber *popupGrabberForDevice(GreenIsland::WlInputDevice *device);

    friend class XdgPopup;
};

}

#endif // XDGSHELL_H
