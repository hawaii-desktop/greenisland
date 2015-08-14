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

#ifndef XDGPOPUP_H
#define XDGPOPUP_H

#include "surface.h"
#include "surfaceinterface.h"

#include "xdgshell.h"

class InputDevice;

namespace GreenIsland {

class ClientWindow;
class XdgPopupGrabber;

class XdgPopup : public QObject, public SurfaceInterface, public QtWaylandServer::xdg_popup
{
public:
    XdgPopup(XdgShell *shell, Surface *parent,
             Surface *surface, InputDevice *device,
             wl_client *client, uint32_t id, uint32_t version, int32_t x, int32_t y,
             uint32_t serial);
    ~XdgPopup();

    XdgPopupGrabber *grabber() const;

    void done();

protected:
    bool runOperation(SurfaceOperation *op) Q_DECL_OVERRIDE;

    void popup_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void popup_destroy(Resource *resource) Q_DECL_OVERRIDE;

private:
    XdgShell *m_shell;
    Surface *m_parentSurface;
    Surface *m_surface;
    ClientWindow *m_window;
    uint32_t m_serial;
    XdgPopupGrabber *m_grabber;

    friend class XdgShell;

private Q_SLOTS:
    void parentSurfaceGone();
    void surfaceMapped();
    void surfaceUnmapped();
    void surfaceConfigured(bool hasBuffer);
};

}

#endif // XDGPOPUP_H
