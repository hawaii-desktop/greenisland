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
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "xdgsurface.h"
#include "xdgsurfacemovegrabber.h"
#include "xdgsurfaceresizegrabber.h"
#include "windowview.h"
#include "output.h"

XdgSurface::XdgSurface(XdgShell *shell, QWaylandSurface *surface,
                       wl_client *client, uint32_t id)
    : QWaylandSurfaceInterface(surface)
    , QtWaylandServer::xdg_surface(client, id)
    , m_shell(shell)
    , m_surface(surface)
    , m_moveGrabber(Q_NULLPTR)
    , m_resizeGrabber(Q_NULLPTR)
    , m_minimized(false)
    , m_state(Normal)
{
    // Create a view for the first output
    QWaylandQuickSurface *quickSurface = static_cast<QWaylandQuickSurface *>(m_surface);
    Output *output = qobject_cast<Output *>(m_surface->compositor()->outputs().at(0));
    m_view = new WindowView(quickSurface, output);

    // This is a toplevel window by default
    m_surface->handle()->setTransientParent(Q_NULLPTR);
    m_surface->handle()->setTransientOffset(0, 0);
    setSurfaceType(QWaylandSurface::Toplevel);

    // Map surface
    connect(m_surface, &QWaylandSurface::configure, [=](bool hasBuffer) {
        m_surface->setMapped(hasBuffer);
    });
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

WindowView *XdgSurface::view() const
{
    return m_view;
}

QQuickItem *XdgSurface::window() const
{
    return m_view->parentItem();
}

WindowView *XdgSurface::parentView() const
{
    QWaylandSurface *transientParent = m_surface->transientParent();
    if (!transientParent)
        return Q_NULLPTR;

    for (QWaylandSurfaceView *surfaceView: transientParent->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;

        if (view->output() == m_view->output())
            return view;
    }

    return Q_NULLPTR;
}

QQuickItem *XdgSurface::parentWindow() const
{
    if (parentView() && parentView()->parentItem())
        return parentView()->parentItem();

    return Q_NULLPTR;
}

void XdgSurface::setPosition(const QPointF &pt)
{
    for (QWaylandSurfaceView *surfaceView: m_surface->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;

        QRectF geometry = view->globalGeometry();
        geometry.setTopLeft(pt);
        view->setGlobalGeometry(geometry);
    }
}

QPointF XdgSurface::transientOffset() const
{
    return m_surface->transientOffset();
}

void XdgSurface::setTransientOffset(const QPointF &pt)
{
    m_surface->handle()->setTransientOffset(pt.x(), pt.y());
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
    changes.active = m_view->hasFocus();
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
}

void XdgSurface::resetResizeGrab()
{
    m_resizeGrabber = Q_NULLPTR;
}

void XdgSurface::requestConfigure(const XdgSurface::Changes &changes)
{
    QByteArray states;
    QDataStream ds(&states, QIODevice::WriteOnly);

    if (changes.newState) {
        if (changes.state == FullScreen)
            ds << QtWaylandServer::xdg_surface::state_fullscreen;
        else if (changes.state == Maximized)
            ds << QtWaylandServer::xdg_surface::state_maximized;
    }

    if (changes.resizing)
        ds << QtWaylandServer::xdg_surface::state_resizing;

    if (changes.active)
        ds << QtWaylandServer::xdg_surface::state_activated;

    uint32_t serial = nextSerial();
    m_pendingChanges[serial] = changes;

    QSizeF size = changes.newState || changes.resizing
            ? changes.size
            : QSizeF(0 ,0);

    send_configure(size.width(), size.height(), states, serial);
}

bool XdgSurface::runOperation(QWaylandSurfaceOp *op)
{
    switch (op->type()) {
    case QWaylandSurfaceOp::Close:
        send_close();
        return true;
    case QWaylandSurfaceOp::Resize: {
        Changes changes;
        changes.active = m_view->hasFocus();
        changes.resizing = true;
        changes.size = QSizeF(static_cast<QWaylandSurfaceResizeOp *>(op)->size());
        requestConfigure(changes);
    }
        return true;
    case QWaylandSurfaceOp::Ping:
        m_shell->pingSurface(this);
        return true;
    case Move:
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
        qWarning() << "Unable to move surface: a move or resize operation was already requested!";
        return;
    }

    // Can't move if the window is maximized or full screen
    if (m_state == Maximized || m_state == FullScreen)
        return;

    // TODO: When maximized we should change state back to normal,
    // restore the size and start the move grab

    QtWayland::Pointer *pointer = device->handle()->pointerDevice();

    m_moveGrabber = new XdgSurfaceMoveGrabber(this, pointer->position() - m_view->globalGeometry().topLeft());
    pointer->startGrab(m_moveGrabber);
}

void XdgSurface::surface_destroy(Resource *resource)
{
    Q_UNUSED(resource);
}

void XdgSurface::surface_set_parent(Resource *resource, wl_resource *parentResource)
{
    Q_UNUSED(resource);

    // Set surface type
    setSurfaceType(QWaylandSurface::Transient);

    // Assign transient parent
    QWaylandSurface *parent = QWaylandSurface::fromResource(parentResource);
    m_surface->handle()->setTransientParent(parent->handle());
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

void XdgSurface::surface_show_window_menu(Resource *resource, wl_resource *seat, uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(seat);
    Q_UNUSED(serial);
    Q_UNUSED(x);
    Q_UNUSED(y);

    // TODO:
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
        qWarning() << "Unable to resize surface: a move or resize operation was already requested!";
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
    Changes changes = m_pendingChanges.take(serial);

    // Set state
    if (changes.active)
        m_view->takeFocus();
    if (changes.newState) {
        m_savedState = m_state;
        m_state = changes.state;
    }

    // Set global space geometry
    bool changed = false;
    QRectF geometry = m_view->globalGeometry();
    if (changes.moving && !changes.position.isNull()) {
        geometry.setTopLeft(changes.position);
        changed = true;
    }
    if (changes.resizing && changes.size.isValid()) {
        geometry.setSize(changes.size);
        changed = true;
    }
    if (changed) {
        // Save global space geometry
        m_savedGeometry = m_view->globalGeometry();

        // Actually change it
        for (QWaylandSurfaceView *surfaceView: m_surface->views()) {
            WindowView *view = static_cast<WindowView *>(surfaceView);
            if (!view)
                continue;
            view->setGlobalGeometry(geometry);
        }
    }
}

void XdgSurface::surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(resource);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);

    // TODO:
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
    QRectF geometry = m_view->mainOutput()->availableGeometry();

    // Ask for a resize on the output where the biggest part of the window
    // is mapped and set pending state, we'll complete the operation as
    // soon as we receive the ACK
    Changes changes;
    changes.newState = true;
    changes.active = m_view->hasFocus();
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
    changes.active = m_view->hasFocus();
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
    Output *output = m_view->mainOutput();
    if (outputResource)
        output = qobject_cast<Output *>(Output::fromResource(outputResource));

    // Ask for a resize on the output and set pending state,
    // we'll complete the operation as soon as we receive the ACK
    Changes changes;
    changes.newState = true;
    changes.active = m_view->hasFocus();
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
    changes.active = m_view->hasFocus();
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

    // Set flag and hide the window representation
    m_minimized = true;
    if (window())
        window()->setVisible(false);
}

#include "moc_xdgsurface.cpp"
