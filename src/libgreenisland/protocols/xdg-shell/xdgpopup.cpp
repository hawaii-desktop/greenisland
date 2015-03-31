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

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "clientwindow.h"
#include "xdgpopup.h"
#include "xdgpopupgrabber.h"

namespace GreenIsland {

XdgPopup::XdgPopup(XdgShell *shell, QWaylandSurface *parent,
                   QWaylandSurface *surface, QWaylandInputDevice *device,
                   wl_client *client, uint32_t id, uint32_t version,
                   int32_t x, int32_t y, uint32_t serial)
    : QObject(shell)
    , QWaylandSurfaceInterface(surface)
    , QtWaylandServer::xdg_popup(client, id, version)
    , m_shell(shell)
    , m_parentSurface(parent)
    , m_surface(surface)
    , m_serial(serial)
    , m_grabber(Q_NULLPTR)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Set surface type
    surface->handle()->setTransientParent(parent->handle());
    surface->handle()->setTransientOffset(x, y);
    setSurfaceType(QWaylandSurface::Popup);

    // Create popup grabber
    m_grabber = shell->popupGrabberForDevice(device->handle());

    // Connect surface signals
    connect(m_parentSurface, &QWaylandSurface::surfaceDestroyed,
            this, &XdgPopup::parentSurfaceGone);
    connect(m_surface, &QWaylandSurface::mapped,
            this, &XdgPopup::surfaceMapped);
    connect(m_surface, &QWaylandSurface::unmapped,
            this, &XdgPopup::surfaceUnmapped);
    connect(m_surface, &QWaylandSurface::configure,
            this, &XdgPopup::surfaceConfigured);

    // Create the window
    m_window = new ClientWindow(m_surface, this);
}

XdgPopup::~XdgPopup()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    m_grabber->removePopup(this);
    m_grabber->m_client = Q_NULLPTR;

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
}

XdgPopupGrabber *XdgPopup::grabber() const
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    return m_grabber;
}

void XdgPopup::done()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    send_popup_done(m_serial);
}

bool XdgPopup::runOperation(QWaylandSurfaceOp *op)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(op)
    return false;
}

void XdgPopup::popup_destroy_resource(Resource *resource)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    delete this;
}

void XdgPopup::popup_destroy(Resource *resource)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

void XdgPopup::parentSurfaceGone()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    done();
    deleteLater();
}

void XdgPopup::surfaceMapped()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Handle popup behavior
    if (m_grabber->serial() == m_serial) {
        m_grabber->addPopup(this);
    } else {
        done();
        m_grabber->m_client = Q_NULLPTR;
    }
}

void XdgPopup::surfaceUnmapped()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Handle popup behavior
    done();
    m_grabber->removePopup(this);
    m_grabber->m_client = Q_NULLPTR;
}

void XdgPopup::surfaceConfigured(bool hasBuffer)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    // Map or unmap the surface
    m_surface->setMapped(hasBuffer);
}

}
