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

#include <QtQuick/QQuickItem>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/QtCompositorVersion>
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "clientwindow.h"
#include "output.h"
#include "windowview.h"
#include "xdgsurface.h"
#include "xdgsurfacemovegrabber.h"
#include "xdgsurfaceresizegrabber.h"

namespace GreenIsland {

XdgSurface::XdgSurface(XdgShell *shell, QWaylandSurface *surface,
                       wl_client *client, uint32_t id)
    : QObject(surface)
    , QWaylandSurfaceInterface(surface)
#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    , QtWaylandServer::xdg_surface(client, id, 1)
#else
    , QtWaylandServer::xdg_surface(client, id)
#endif
    , m_shell(shell)
    , m_surface(surface)
    , m_moveGrabber(Q_NULLPTR)
    , m_resizeGrabber(Q_NULLPTR)
    , m_minimized(false)
    , m_state(Normal)
    , m_savedState(Normal)
    , m_deleting(false)
{
    // This is a toplevel window by default
    m_surface->handle()->setTransientParent(Q_NULLPTR);
    m_surface->handle()->setTransientOffset(0, 0);
    setSurfaceType(QWaylandSurface::Toplevel);

    // Create client window
    m_window = new ClientWindow(surface, this);

    // Tell the client when this window is active
    connect(m_window, &ClientWindow::activeChanged, [=] {
        Changes changes;
        changes.newState = false;
        changes.active = m_window->isActive();
        changes.moving = false;
        changes.resizing = false;
        requestConfigure(changes);
    });

    // Surface events
    connect(m_surface, &QWaylandSurface::configure, [=](bool hasBuffer) {
        // Map or unmap the surface
        m_surface->setMapped(hasBuffer);
    });
}

XdgSurface::~XdgSurface()
{
    // Destroy the resource here but don't do it if the destructor is
    // called by shell_surface_destroy_resource() which happens when
    // the resource is destroyed
    if (!m_deleting) {
        m_deleting = true;
        wl_resource_destroy(resource()->handle);
    }
}

uint32_t XdgSurface::nextSerial() const
{
    return wl_display_next_serial(m_surface->handle()->compositor()->display()->handle());
}

QWaylandSurface::WindowType XdgSurface::type() const
{
    return m_surface->windowType();
}

XdgSurface::State XdgSurface::state() const
{
    return m_state;
}

QWaylandSurface *XdgSurface::surface() const
{
    return m_surface;
}

ClientWindow *XdgSurface::window() const
{
    return m_window;
}

void XdgSurface::restore()
{
    restoreAt(m_savedGeometry.topLeft());
}

void XdgSurface::restoreAt(const QPointF &pos)
{
    // Makes sense only for maximized windows
    if (m_state == Normal)
        return;

    // Restore previous geometry
    Changes changes;
    changes.newState = true;
    changes.active = m_window->isActive();
    changes.state = Normal;
    changes.moving = true;
    changes.resizing = true;
    changes.position = pos;
    changes.size = m_savedGeometry.size();
    requestConfigure(changes);
}

void XdgSurface::resetMoveGrab()
{
    m_moveGrabber = Q_NULLPTR;

    // Notify that motion has finished (a QML shell might want to enable
    // x,y animations again)
    Q_EMIT m_window->motionFinished();
}

void XdgSurface::resetResizeGrab()
{
    m_resizeGrabber = Q_NULLPTR;
}

void XdgSurface::requestConfigure(const XdgSurface::Changes &changes)
{
    struct wl_array states;
    uint32_t *s;

    wl_array_init(&states);

    if (changes.newState) {
        if (changes.state == FullScreen) {
            s = (uint32_t*)wl_array_add(&states, sizeof *s);
            *s = QtWaylandServer::xdg_surface::state_fullscreen;
        } else if (changes.state == Maximized) {
            s = (uint32_t*)wl_array_add(&states, sizeof *s);
            *s = QtWaylandServer::xdg_surface::state_maximized;
        }
    }

    if (changes.resizing) {
        s = (uint32_t*)wl_array_add(&states, sizeof *s);
        *s = QtWaylandServer::xdg_surface::state_resizing;
    }

    if (changes.active) {
        s = (uint32_t*)wl_array_add(&states, sizeof *s);
        *s = QtWaylandServer::xdg_surface::state_activated;
    }

    uint32_t serial = nextSerial();
    m_pendingChanges[serial] = changes;

    QSizeF size = changes.newState || changes.resizing
            ? changes.size
            : QSizeF(0 ,0);

    QByteArray statesArray((const char *)states.data, states.size);
    send_configure(size.width(), size.height(), statesArray, serial);

    wl_array_release(&states);
}

bool XdgSurface::runOperation(QWaylandSurfaceOp *op)
{
    switch (op->type()) {
    case QWaylandSurfaceOp::Resize: {
        Changes changes;
        changes.active = m_window->isActive();
        changes.resizing = true;
        changes.size = QSizeF(static_cast<QWaylandSurfaceResizeOp *>(op)->size());
        requestConfigure(changes);
    }
        return true;
    case QWaylandSurfaceOp::Ping:
        m_shell->pingSurface(this);
        return true;
    case ClientWindow::Move:
        moveWindow(m_surface->compositor()->defaultInputDevice());
        return true;
    default:
        break;
    }

    return false;
}

void XdgSurface::moveWindow(QWaylandInputDevice *device)
{
    if (m_moveGrabber || m_resizeGrabber) {
        qCWarning(XDGSHELL_PROTOCOL) << "Unable to move surface: a move or resize operation was already requested!";
        return;
    }

    // Can't move if the window is maximized or full screen
    if (m_state == Maximized || m_state == FullScreen)
        return;

    // TODO: When maximized we should change state back to normal,
    // restore the size and start the move grab

    QtWayland::Pointer *pointer = device->handle()->pointerDevice();

    m_moveGrabber = new XdgSurfaceMoveGrabber(this, pointer->position() - m_window->position());
    pointer->startGrab(m_moveGrabber);

    // Notify that motion is starting (a QML shell might want to disable x,y animations
    // to make the movement smoother)
    Q_EMIT m_window->motionStarted();
}

void XdgSurface::surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);

    // Don't delete twice if we are here from the destructor
    if (!m_deleting) {
        m_deleting = true;
        deleteLater();
    }
}

void XdgSurface::surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgSurface::surface_set_parent(Resource *resource, wl_resource *parentResource)
{
    Q_UNUSED(resource);

    // Assign transient parent
    if (parentResource) {
        // Set surface type
        setSurfaceType(QWaylandSurface::Transient);

        QWaylandSurface *parent = QWaylandSurface::fromResource(parentResource);
        m_surface->handle()->setTransientParent(parent->handle());
    } else {
        // Some applications such as gnome-cloks send a set_parent(nil), in
        // this case we assume it's a toplevel window
        setSurfaceType(QWaylandSurface::Toplevel);
    }
}

void XdgSurface::surface_set_title(Resource *resource, const QString &title)
{
    Q_UNUSED(resource);

    setSurfaceTitle(title);
}

void XdgSurface::surface_set_app_id(Resource *resource, const QString &app_id)
{
    Q_UNUSED(resource);

    setSurfaceClassName(app_id);
}

void XdgSurface::surface_show_window_menu(Resource *resource, wl_resource *seat,
                                          uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(seat);
    Q_UNUSED(serial);

    Q_EMIT m_window->windowMenuRequested(QPoint(x, y));
}

void XdgSurface::surface_move(Resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    moveWindow(QtWayland::InputDevice::fromSeatResource(seat)->handle());
}

void XdgSurface::surface_resize(Resource *resource, wl_resource *seat, uint32_t serial, uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    if (m_moveGrabber || m_resizeGrabber) {
        qCWarning(XDGSHELL_PROTOCOL) << "Unable to resize surface: a move or resize operation was already requested!";
        return;
    }

    // Can't resize if the window is maximized or full screen
    if (m_state == Maximized || m_state == FullScreen)
        return;

    m_resizeGrabber = new XdgSurfaceResizeGrabber(this);

    QtWayland::InputDevice *device = QtWayland::InputDevice::fromSeatResource(seat);
    QtWayland::Pointer *pointer = device->pointerDevice();

    m_resizeGrabber->m_pt = pointer->position();
    m_resizeGrabber->m_resizeEdges = static_cast<resize_edge>(edges);
    m_resizeGrabber->m_width = m_surface->size().width();
    m_resizeGrabber->m_height = m_surface->size().height();

    pointer->startGrab(m_resizeGrabber);
}

void XdgSurface::surface_ack_configure(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);

    // Surface was configured, now we can set the new state
    if (m_pendingChanges.find(serial) == m_pendingChanges.end())
        return;
    Changes changes = m_pendingChanges.take(serial);

    // Set state
    if (changes.newState) {
        m_savedState = m_state;
        m_state = changes.state;

        switch (m_state) {
        case Normal:
            m_window->unmaximize();
            m_window->setFullScreen(false);
            break;
        case Maximized:
            m_window->maximize();
            break;
        case FullScreen:
            m_window->setFullScreen(true);
            break;
        default:
            break;
        }
    }

    // Save previous geometry and move window
    if (changes.moving) {
        m_savedGeometry = m_window->geometry();
        m_window->setPosition(changes.position);
    }
}

void XdgSurface::surface_set_window_geometry(Resource *resource,
                                             int32_t x, int32_t y,
                                             int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    m_window->setInternalGeometry(QRectF(QPointF(x, y), QSizeF(width, height)));
}

void XdgSurface::surface_set_maximized(Resource *resource)
{
    Q_UNUSED(resource);

    // Only top level windows can be maximized
    if (m_surface->windowType() != QWaylandSurface::Toplevel)
        return;

    // Ignore if already maximized
    if (m_state == Maximized)
        return;

    // New global space geometry
    QRectF geometry = m_window->output()->availableGeometry();

    // Ask for a resize on the output where the biggest part of the window
    // is mapped and set pending state, we'll complete the operation as
    // soon as we receive the ACK
    Changes changes;
    changes.newState = true;
    changes.active = m_window->isActive();
    changes.state = Maximized;
    changes.moving = true;
    changes.resizing = true;
    changes.position = geometry.topLeft();
    changes.size = geometry.size();
    requestConfigure(changes);
}

void XdgSurface::surface_unset_maximized(Resource *resource)
{
    Q_UNUSED(resource);

    // Set state back to normal and restore geometry when
    // ACK is received
    Changes changes;
    changes.newState = true;
    changes.active = m_window->isActive();
    changes.state = m_savedState;
    changes.moving = true;
    changes.resizing = true;
    changes.position = m_savedGeometry.topLeft();
    changes.size = m_savedGeometry.size();
    requestConfigure(changes);
}

void XdgSurface::surface_set_fullscreen(Resource *resource, wl_resource *outputResource)
{
    Q_UNUSED(resource);

    // Only top level windows can be full screen
    if (m_surface->windowType() != QWaylandSurface::Toplevel)
        return;

    // Ignore if already full screen
    if (m_state == FullScreen)
        return;

    // Determine output (either specified output or main output)
    QWaylandOutput *output = m_window->output();
    if (outputResource)
        output = QWaylandOutput::fromResource(outputResource);

    // Ask for a resize on the output and set pending state,
    // we'll complete the operation as soon as we receive the ACK
    Changes changes;
    changes.newState = true;
    changes.active = m_window->isActive();
    changes.state = FullScreen;
    changes.moving = true;
    changes.resizing = true;
    changes.position = output->geometry().topLeft();
    changes.size = output->geometry().size();
    requestConfigure(changes);
}

void XdgSurface::surface_unset_fullscreen(Resource *resource)
{
    Q_UNUSED(resource);

    // Set state back to normal and restore geometry when
    // ACK is received
    Changes changes;
    changes.newState = true;
    changes.active = m_window->isActive();
    changes.state = m_savedState;
    changes.moving = true;
    changes.resizing = true;
    changes.position = m_savedGeometry.topLeft();
    changes.size = m_savedGeometry.size();
    requestConfigure(changes);
}

void XdgSurface::surface_set_minimized(Resource *resource)
{
    Q_UNUSED(resource);

    m_minimized = true;
    m_window->minimize();
}

}

#include "moc_xdgsurface.cpp"
