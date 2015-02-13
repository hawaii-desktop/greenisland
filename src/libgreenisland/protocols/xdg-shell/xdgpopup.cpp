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

#include <QtCompositor/QtCompositorVersion>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "clientwindow.h"
#include "output.h"
#include "xdgpopup.h"
#include "xdgpopupgrabber.h"

namespace GreenIsland {

XdgPopup::XdgPopup(XdgShell *shell, QWaylandSurface *parent, QWaylandSurface *surface,
                   wl_client *client, uint32_t id, uint32_t serial)
    : QWaylandSurfaceInterface(surface)
#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    , QtWaylandServer::xdg_popup(client, id, 1)
#else
    , QtWaylandServer::xdg_popup(client, id)
#endif
    , m_shell(shell)
    , m_parentSurface(parent)
    , m_surface(surface)
    , m_serial(serial)
    , m_grabber(Q_NULLPTR)
    , m_deleting(false)
{
    // Set surface type
    setSurfaceType(QWaylandSurface::Popup);

    // Create the window
    m_window = new ClientWindow(m_surface, this);

    // Surface mapping and unmapping
    connect(m_surface, &QWaylandSurface::mapped, [=]() {
        // Handle popup behavior
        if (m_grabber->serial() == m_serial) {
            m_grabber->addPopup(this);
        } else {
            done();
            m_grabber->m_client = Q_NULLPTR;
        }
    });
    connect(m_surface, &QWaylandSurface::unmapped, [=]() {
        // Handle popup behavior
        done();
        m_grabber->removePopup(this);
        m_grabber->m_client = Q_NULLPTR;
    });
    connect(m_surface, &QWaylandSurface::configure, [=](bool hasBuffer) {
        // Map or unmap the surface
        m_surface->setMapped(hasBuffer);
    });
    connect(parent, SIGNAL(unmapped()), m_surface, SIGNAL(unmapped()));
}

XdgPopup::~XdgPopup()
{
    // Destroy the resource here but don't do it if the destructor is
    // called by shell_surface_destroy_resource() which happens when
    // the resource is destroyed
    if (!m_deleting) {
        m_deleting = true;
        wl_resource_destroy(resource()->handle);
    }
}

XdgPopupGrabber *XdgPopup::grabber() const
{
    return m_grabber;
}

void XdgPopup::done()
{
    send_popup_done(m_serial);
}

bool XdgPopup::runOperation(QWaylandSurfaceOp *op)
{
    return false;
}

void XdgPopup::popup_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);

    // Don't delete twice if we are here from the destructor
    if (!m_deleting) {
        m_deleting = true;
        deleteLater();
    }
}

void XdgPopup::popup_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

}

#include "moc_xdgpopup.cpp"
