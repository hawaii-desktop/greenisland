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

#ifndef XDGPOPUP_H
#define XDGPOPUP_H

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceInterface>

#include "xdgshell.h"

namespace GreenIsland {

class XdgPopupGrabber;

class XdgPopup : public QObject, public QWaylandSurfaceInterface, public QtWaylandServer::xdg_popup
{
    Q_OBJECT
public:
    explicit XdgPopup(XdgShell *shell, QWaylandSurface *parent, QWaylandSurface *surface,
                      wl_client *client, uint32_t id, uint32_t serial);

    XdgPopupGrabber *grabber() const;

    void done();

protected:
    bool runOperation(QWaylandSurfaceOp *op);

private:
    XdgShell *m_shell;
    QWaylandSurface *m_parentSurface;
    QWaylandSurface *m_surface;
    uint32_t m_serial;
    XdgPopupGrabber *m_grabber;

    friend class XdgShell;
};

}

#endif // XDGPOPUP_H
