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

#include <GreenIsland/Compositor/QWaylandCompositor>
#include <GreenIsland/Compositor/QWaylandInput>
#include <waylandcompositor/wayland_wrapper/qwlcompositor_p.h>
#include <waylandcompositor/wayland_wrapper/qwlinputdevice_p.h>
#include <waylandcompositor/wayland_wrapper/qwlpointer_p.h>
#include <waylandcompositor/wayland_wrapper/qwlsurface_p.h>

#include "clientwindow.h"
#include "output.h"
#include "wlshellsurface.h"
#include "wlshellsurfacemovegrabber.h"
#include "wlshellsurfaceresizegrabber.h"
#include "wlshellsurfacepopupgrabber.h"
#include "windowview.h"

namespace GreenIsland {

WlShellSurface::WlShellSurface(WlShell *shell, QWaylandSurface *surface,
                               wl_client *client, uint32_t id, uint32_t version)
    : QObject(shell)
    , QWaylandSurfaceInterface(surface)
    , QtWaylandServer::wl_shell_surface(client, id, version)
    , m_shell(shell)
    , m_surface(surface)
    , m_moveGrabber(Q_NULLPTR)
    , m_resizeGrabber(Q_NULLPTR)
    , m_popupGrabber(Q_NULLPTR)
    , m_popupSerial()
    , m_state(Normal)
    , m_prevState(Normal)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Create client window
    m_window = new ClientWindow(surface, this);

    // Surface events
    connect(m_surface, &QWaylandSurface::configure, this, [this](bool hasBuffer) {
        // Map or unmap the surface
        m_surface->setMapped(hasBuffer);
    });
    connect(m_surface, &QWaylandSurface::mapped, this, [this]() {
        // Popup behavior
        if (m_surface->windowType() == QWaylandSurface::Popup) {
            if (m_popupGrabber->serial() == m_popupSerial) {
                m_popupGrabber->addPopup(this);
            } else {
                send_popup_done();
                m_popupGrabber->m_client = Q_NULLPTR;
            }
        }
    });
}

WlShellSurface::~WlShellSurface()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    delete m_moveGrabber;
    delete m_resizeGrabber;

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
    //surface()->setMapped(false);
}

WlShellSurface::State WlShellSurface::state() const
{
    return m_state;
}

QWaylandSurface *WlShellSurface::surface() const
{
    return m_surface;
}

ClientWindow *WlShellSurface::window() const
{
    return m_window;
}

void WlShellSurface::restore()
{
    restoreAt(m_prevGlobalGeometry.topLeft());
}

void WlShellSurface::restoreAt(const QPointF &pos)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    // Makes sense only for maximized windows
    if (m_state == Normal)
        return;

    // Restore previous state and position
    m_prevState = m_state;
    m_state = Normal;
    if (m_window) {
        m_window->setPosition(pos);
        m_window->unmaximize();
        m_window->setFullScreen(false);
    }

    // Actually resize it
    requestResize(m_prevGlobalGeometry.size().toSize());
}

void WlShellSurface::resetMoveGrab()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    if (!m_moveGrabber)
        return;

    delete m_moveGrabber;
    m_moveGrabber = Q_NULLPTR;

    // Notify that motion has finished (a QML shell might want to enable
    // x,y animations again)
    if (m_window)
        Q_EMIT m_window->motionFinished();
}

void WlShellSurface::resetResizeGrab()
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    if (!m_resizeGrabber)
        return;

    delete m_resizeGrabber;
    m_resizeGrabber = Q_NULLPTR;

    // Notify that resize has finished (a QML shell might want to enable
    // width and height animations again)
    if (m_window)
        Q_EMIT m_window->resizeFinished();
}

bool WlShellSurface::runOperation(QWaylandSurfaceOp *op)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;
    qCDebug(WLSHELL_TRACE) << "Run operation" << op->type();

    switch (op->type()) {
    case QWaylandSurfaceOp::Ping:
        ping(static_cast<QWaylandSurfacePingOp *>(op)->serial());
        return true;
    case QWaylandSurfaceOp::Resize:
        requestResize(static_cast<QWaylandSurfaceResizeOp *>(op)->size());
        return true;
    case QWaylandSurfaceOp::Close:
        m_surface->compositor()->handle()->destroySurface(m_surface->handle());
        return true;
    case ClientWindow::Move:
        moveWindow(m_surface->compositor()->defaultInputDevice());
        return true;
    case ClientWindow::StopMove:
        if (m_moveGrabber) {
            QWaylandInputDevice *device = m_surface->compositor()->defaultInputDevice();
            QtWayland::Pointer *pointer = device->handle()->pointerDevice();
            pointer->sendButton(0, Qt::LeftButton, 0);
        }
        return true;
    default:
        break;
    }

    return false;
}

void WlShellSurface::shell_surface_destroy_resource(Resource *resource)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    // Close popup grabber in case it is still going
    if (m_popupGrabber) {
        m_popupGrabber->removePopup(this);
        m_popupGrabber->m_client = Q_NULLPTR;
    }

    delete this;
}

void WlShellSurface::shell_surface_pong(Resource *resource, uint32_t serial)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    if (m_pings.remove(serial))
        Q_EMIT m_surface->pong();
    else
        qCWarning(WLSHELL_PROTOCOL) << "Received unexpected pong with serial" << serial;
}

void WlShellSurface::shell_surface_move(Resource *resource, wl_resource *seat,
                                        uint32_t serial)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    Q_UNUSED(serial)

    moveWindow(QtWayland::InputDevice::fromSeatResource(seat)->handle());
}

void WlShellSurface::shell_surface_resize(Resource *resource, wl_resource *seat,
                                          uint32_t serial, uint32_t edges)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    Q_UNUSED(serial)

    if (m_moveGrabber || m_resizeGrabber) {
        qCWarning(WLSHELL_PROTOCOL) << "Unable to resize surface: a move or resize operation was already requested!";
        return;
    }

    // Can't resize if the window is maximized or full screen
    if (m_state == Maximized || m_state == FullScreen)
        return;

    // Check for invalid edge combination
    const unsigned resize_topbottom =
            QtWaylandServer::wl_shell_surface::resize_top |
            QtWaylandServer::wl_shell_surface::resize_bottom;
    const unsigned resize_leftright =
            QtWaylandServer::wl_shell_surface::resize_left |
            QtWaylandServer::wl_shell_surface::resize_right;
    const unsigned resize_any = resize_topbottom | resize_leftright;
    if (edges == QtWaylandServer::wl_shell_surface::resize_none ||
            edges > resize_any ||
            (edges & resize_topbottom) == resize_topbottom ||
            (edges & resize_leftright) == resize_leftright)
        return;

    m_resizeGrabber = new WlShellSurfaceResizeGrabber(this);

    QtWayland::InputDevice *device = QtWayland::InputDevice::fromSeatResource(seat);
    QtWayland::Pointer *pointer = device->pointerDevice();

    m_resizeGrabber->m_pt = pointer->position();
    m_resizeGrabber->m_resizeEdges = static_cast<resize>(edges);
    m_resizeGrabber->m_width = m_surface->size().width();
    m_resizeGrabber->m_height = m_surface->size().height();

    pointer->startGrab(m_resizeGrabber);

    // Notify that resize is starting (a QML shell might want to disable
    // width and height animations to make the movement smoother)
    if (m_window)
        Q_EMIT m_window->resizeStarted();
}

void WlShellSurface::shell_surface_set_toplevel(Resource *resource)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    m_surface->handle()->setTransientParent(Q_NULLPTR);
    m_surface->handle()->setTransientOffset(0, 0);

    setSurfaceType(QWaylandSurface::Toplevel);

    m_surface->setVisibility(QWindow::Windowed);

    // Check whether the window has gone away
    if (!m_window)
        return;

    // Restore state and geometry if it was maximized or full screen
    if (m_state == Maximized || m_state == FullScreen) {
        m_prevState = m_state;
        m_state = Normal;
        m_window->setPosition(m_prevGlobalGeometry.topLeft());
        requestResize(m_prevGlobalGeometry.size().toSize());
        m_window->unmaximize();
        m_window->setFullScreen(false);
    }
}

void WlShellSurface::shell_surface_set_transient(Resource *resource, wl_resource *parentResource,
                                                 int32_t x, int32_t y, uint32_t flags)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    QWaylandSurface *parentSurface = QWaylandSurface::fromResource(parentResource);
    m_surface->handle()->setTransientParent(parentSurface->handle());
    m_surface->handle()->setTransientOffset(x, y);

    if (flags & QtWaylandServer::wl_shell_surface::transient_inactive)
        m_surface->handle()->setTransientInactive(true);

    setSurfaceType(QWaylandSurface::Transient);

    m_surface->setVisibility(QWindow::AutomaticVisibility);
}

void WlShellSurface::shell_surface_set_fullscreen(Resource *resource, uint32_t method,
                                                  uint32_t framerate, wl_resource *outputResource)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    Q_UNUSED(method)
    Q_UNUSED(framerate)

    // Check whether the window has gone away
    if (!m_window)
        return;

    QWaylandOutput *output = outputResource
            ? QWaylandOutput::fromResource(outputResource)
            : m_window->output();

    // Save global geometry before resizing, it will be restored with the next
    // set_toplevel() call but if the window starts full screen we don't have
    // a valid previous geometry so we set it to output geometry
    if (m_window->geometry().isValid())
        m_prevGlobalGeometry = m_window->geometry();
    else
        m_prevGlobalGeometry = output->geometry();

    // Change global geometry for all views, this will result in
    // moving the window and set a size that accomodate the surface
    m_window->setPosition(QPointF(output->geometry().topLeft()));
    qCDebug(WLSHELL_PROTOCOL) << "Set position to" << m_window->position();
    requestResize(output->geometry().size());
    qCDebug(WLSHELL_PROTOCOL) << "Request resize to" << output->geometry().size();

    m_surface->setVisibility(QWindow::FullScreen);

    // Set state
    m_prevState = m_state;
    m_state = FullScreen;
    m_window->setFullScreen(true);
}

void WlShellSurface::shell_surface_set_popup(Resource *resource, wl_resource *seat,
                                             uint32_t serial, wl_resource *parent,
                                             int32_t x, int32_t y, uint32_t flags)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)
    Q_UNUSED(flags)

    QtWayland::InputDevice *device = QtWayland::InputDevice::fromSeatResource(seat);

    m_popupGrabber = m_shell->popupGrabberForDevice(device);
    m_popupSerial = serial;

    m_surface->handle()->setTransientParent(QtWayland::Surface::fromResource(parent));
    m_surface->handle()->setTransientOffset(x, y);

    setSurfaceType(QWaylandSurface::Popup);

    m_surface->setVisibility(QWindow::AutomaticVisibility);
}

void WlShellSurface::shell_surface_set_maximized(Resource *resource, wl_resource *outputResource)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    // Only top level windows can be maximized
    if (m_surface->windowType() != QWaylandSurface::Toplevel)
        return;

    // Ignore if already maximized
    if (m_state == Maximized)
        return;

    // Check whether the window has gone away
    if (!m_window)
        return;

    QWaylandOutput *output = outputResource
            ? QWaylandOutput::fromResource(outputResource)
            : m_window->output();

    // Save global geometry before resizing, it will be restored with the next
    // set_toplevel() call but if the window starts maximized we don't have
    // a valid previous geometry so we set it to output available geometry
    if (m_window->geometry().isValid())
        m_prevGlobalGeometry = m_window->geometry();
    else
        m_prevGlobalGeometry = output->availableGeometry();

    // Maximize for this output
    m_window->maximize(static_cast<Output *>(output));

    m_surface->setVisibility(QWindow::Maximized);

    // Set state
    m_prevState = m_state;
    m_state = Maximized;
}

void WlShellSurface::shell_surface_set_title(Resource *resource, const QString &title)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    setSurfaceTitle(title);
}

void WlShellSurface::shell_surface_set_class(Resource *resource, const QString &class_)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    Q_UNUSED(resource)

    setSurfaceClassName(class_);
}

void WlShellSurface::ping(uint32_t serial)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    m_pings.insert(serial);
    send_ping(serial);
}

void WlShellSurface::moveWindow(QWaylandInputDevice *device)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    if (m_moveGrabber || m_resizeGrabber) {
        qCWarning(WLSHELL_PROTOCOL) << "Unable to move surface: a move or resize operation was already requested!";
        return;
    }

    // Can't move if the window is full screen
    if (m_state == FullScreen)
        return;

    // Check whether the window has gone away
    if (!m_window)
        return;

    QtWayland::Pointer *pointer = device->handle()->pointerDevice();

    m_moveGrabber = new WlShellSurfaceMoveGrabber(this, pointer->position() - m_window->position());
    pointer->startGrab(m_moveGrabber);

    // Notify that motion is starting (a QML shell might want to disable x,y animations
    // to make the movement smoother)
    Q_EMIT m_window->motionStarted();
}

void WlShellSurface::requestResize(const QSize &size)
{
    qCDebug(WLSHELL_TRACE) << Q_FUNC_INFO;

    send_configure(resize_bottom_right, size.width(), size.height());
}

}
