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
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/private/qwlextendedsurface_p.h>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlpointer_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "wlshellsurface.h"
#include "wlshellsurfacemovegrabber.h"
#include "wlshellsurfaceresizegrabber.h"
#include "wlshellsurfacepopupgrabber.h"
#include "windowview.h"
#include "output.h"

WlShellSurface::WlShellSurface(WlShell *shell, QWaylandSurface *surface,
                               wl_client *client, uint32_t id)
    : QWaylandSurfaceInterface(surface)
    , QtWaylandServer::wl_shell_surface(client, id)
    , m_shell(shell)
    , m_surface(surface)
    , m_moveGrabber(Q_NULLPTR)
    , m_resizeGrabber(Q_NULLPTR)
    , m_popupGrabber(Q_NULLPTR)
    , m_state(Normal)
    , m_prevState(Normal)
    , m_deleting(false)
{
    // Create a view for the first output
    QWaylandQuickSurface *quickSurface = static_cast<QWaylandQuickSurface *>(m_surface);
    Output *output = qobject_cast<Output *>(m_surface->compositor()->outputs().at(0));
    m_view = new WindowView(quickSurface, output);

    // Map surface
    connect(m_surface, &QWaylandSurface::configure, [=](bool hasBuffer) {
        m_surface->setMapped(hasBuffer);
    });
}

WlShellSurface::~WlShellSurface()
{
    // Destroy the resource here but don't do it if the destructor is
    // called by shell_surface_destroy_resource() which happens when
    // the resource is destroyed
    if (!m_deleting) {
        m_deleting = true;
        wl_resource_destroy(resource()->handle);
    }
}

WindowView *WlShellSurface::view() const
{
    return m_view;
}

QQuickItem *WlShellSurface::window() const
{
    return m_view->parentItem();
}

QQuickItem *WlShellSurface::transientParent() const
{
    QWaylandSurface *transientParent = m_surface->transientParent();
    if (!transientParent)
        return Q_NULLPTR;

    for (QWaylandSurfaceView *surfaceView: transientParent->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;

        if (view->output() == m_view->output())
            return view->parentItem();
    }

    return Q_NULLPTR;
}

void WlShellSurface::setPosition(const QPointF &pt)
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

void WlShellSurface::setOffset(const QPointF &pt)
{
    m_surface->handle()->setTransientOffset(pt.x(), pt.y());
}

bool WlShellSurface::runOperation(QWaylandSurfaceOp *op)
{
    switch (op->type()) {
    case QWaylandSurfaceOp::Close:
        if (m_surface->windowType() == QWaylandSurface::Popup) {
            send_popup_done();
            m_popupGrabber->m_client = Q_NULLPTR;
            return true;
        }
        return false;
    case QWaylandSurfaceOp::Ping:
        ping(static_cast<QWaylandSurfacePingOp *>(op)->serial());
        return true;
    case QWaylandSurfaceOp::Resize:
        requestResize(static_cast<QWaylandSurfaceResizeOp *>(op)->size());
        return true;
    case Move:
        moveWindow(m_surface->compositor()->defaultInputDevice());
        return true;
    default:
        break;
    }

    return false;
}

void WlShellSurface::ping(uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(serial);
}

void WlShellSurface::moveWindow(QWaylandInputDevice *device)
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

    // Convert pointer coordinates to global space
    QPointF pos(m_view->globalGeometry().topLeft() + pointer->currentPosition());

    m_moveGrabber = new WlShellSurfaceMoveGrabber(this, pos - m_view->globalGeometry().topLeft());
    pointer->startGrab(m_moveGrabber);
}

void WlShellSurface::requestResize(const QSize &size)
{
    send_configure(resize_bottom_right, size.width(), size.height());
}

void WlShellSurface::shell_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);

    // Close popup grabber in case it is still going
    if (m_popupGrabber)
        m_popupGrabber->removePopup(this);

    // Don't delete twice if we are here from the destructor
    if (!m_deleting) {
        m_deleting = true;
        delete this;
    }
}

void WlShellSurface::shell_surface_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);

    if (m_pings.remove(serial))
        Q_EMIT m_surface->pong();
    else
        qWarning() << "Received unexpected pong with serial" << serial;
}

void WlShellSurface::shell_surface_move(Resource *resource, wl_resource *seat,
                                        uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    moveWindow(QtWayland::InputDevice::fromSeatResource(seat)->handle());
}

void WlShellSurface::shell_surface_resize(Resource *resource, wl_resource *seat,
                                          uint32_t serial, uint32_t edges)
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

    m_resizeGrabber = new WlShellSurfaceResizeGrabber(this);

    QtWayland::InputDevice *device = QtWayland::InputDevice::fromSeatResource(seat);
    QtWayland::Pointer *pointer = device->pointerDevice();

    m_resizeGrabber->m_pt = pointer->position();
    m_resizeGrabber->m_resizeEdges = static_cast<resize>(edges);
    m_resizeGrabber->m_width = m_surface->size().width();
    m_resizeGrabber->m_height = m_surface->size().height();

    pointer->startGrab(m_resizeGrabber);
}

void WlShellSurface::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);

    m_surface->handle()->setTransientParent(Q_NULLPTR);
    m_surface->handle()->setTransientOffset(0, 0);

    setSurfaceType(QWaylandSurface::Toplevel);

    // Restore state and geometry if it was maximized or full screen
    if (m_state == Maximized || m_state == FullScreen) {
        m_prevState = m_state;
        m_state = Normal;

        for (QWaylandSurfaceView *surfaceView: m_surface->views()) {
            WindowView *view = static_cast<WindowView *>(surfaceView);
            if (!view)
                continue;
            qDebug() << view->globalGeometry() << m_prevGlobalGeometry;
            view->setGlobalGeometry(m_prevGlobalGeometry);
        }
    }

    if (m_surface->handle()->extendedSurface())
        m_surface->handle()->extendedSurface()->setVisibility(QWindow::Windowed, false);
}

void WlShellSurface::shell_surface_set_transient(Resource *resource, wl_resource *parentResource,
                                                 int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);

    QWaylandSurface *parentSurface = QWaylandSurface::fromResource(parentResource);
    m_surface->handle()->setTransientParent(parentSurface->handle());
    m_surface->handle()->setTransientOffset(x, y);

    if (flags & QtWaylandServer::wl_shell_surface::transient_inactive)
        m_surface->handle()->setTransientInactive(true);

    setSurfaceType(QWaylandSurface::Transient);

    if (m_surface->handle()->extendedSurface())
        m_surface->handle()->extendedSurface()->setVisibility(QWindow::AutomaticVisibility, false);
}

void WlShellSurface::shell_surface_set_fullscreen(Resource *resource, uint32_t method,
                                                  uint32_t framerate, wl_resource *outputResource)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);

    // Save global geometry before resizing, it will be restored
    // with the next set_toplevel() call
    m_prevGlobalGeometry = m_view->globalGeometry();

    QWaylandOutput *output = outputResource
            ? QWaylandOutput::fromResource(outputResource)
            : m_view->mainOutput();

    // Resize
    send_configure(resize_bottom_right,
                   output->geometry().size().width(),
                   output->geometry().size().height());

    // Change global geometry for all views, this will result in
    // moving the window and set a size that accomodate the surface
    for (QWaylandSurfaceView *surfaceView: m_surface->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;
        view->setGlobalGeometry(output->geometry());
    }

    // Set state
    m_prevState = m_state;
    m_state = FullScreen;

    if (m_surface->handle()->extendedSurface())
        m_surface->handle()->extendedSurface()->setVisibility(QWindow::FullScreen, false);
}

void WlShellSurface::shell_surface_set_popup(Resource *resource, wl_resource *seat,
                                             uint32_t serial, wl_resource *parent,
                                             int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(flags);

    QtWayland::InputDevice *device = QtWayland::InputDevice::fromSeatResource(seat);

    m_popupGrabber = m_shell->popupGrabberForDevice(device);

    m_surface->handle()->setTransientParent(QtWayland::Surface::fromResource(parent));
    m_surface->handle()->setTransientOffset(x, y);

    setSurfaceType(QWaylandSurface::Popup);

    if (m_surface->handle()->extendedSurface())
        m_surface->handle()->extendedSurface()->setVisibility(QWindow::AutomaticVisibility, false);

    // Map popup
    connect(m_surface, &QWaylandSurface::mapped, [=]() {
        if (m_surface->handle()->mapped() &&
                m_popupGrabber &&
                m_popupGrabber->serial() == serial) {
            m_popupGrabber->addPopup(this);
        } else {
            send_popup_done();
            m_popupGrabber->m_client = Q_NULLPTR;
        }
    });
}

void WlShellSurface::shell_surface_set_maximized(Resource *resource, wl_resource *outputResource)
{
    Q_UNUSED(resource);

    // Save global geometry before resizing, it will be restored
    // with the next set_toplevel() call
    m_prevGlobalGeometry = m_view->globalGeometry();

    QWaylandOutput *output = outputResource
            ? QWaylandOutput::fromResource(outputResource)
            : m_view->mainOutput();

    // Resize
    send_configure(resize_bottom_right,
                   output->availableGeometry().size().width(),
                   output->availableGeometry().size().height());

    // Change global geometry for all views, this will result in
    // moving the window and set a size that accomodate the surface
    for (QWaylandSurfaceView *surfaceView: m_surface->views()) {
        WindowView *view = static_cast<WindowView *>(surfaceView);
        if (!view)
            continue;
        view->setGlobalGeometry(output->availableGeometry());
    }

    // Set state
    m_prevState = m_state;
    m_state = Maximized;

    if (m_surface->handle()->extendedSurface())
        m_surface->handle()->extendedSurface()->setVisibility(QWindow::Maximized, false);
}

void WlShellSurface::shell_surface_set_title(Resource *resource, const QString &title)
{
    Q_UNUSED(resource);

    setSurfaceTitle(title);
}

void WlShellSurface::shell_surface_set_class(Resource *resource, const QString &class_)
{
    Q_UNUSED(resource);

    setSurfaceClassName(class_);
}
