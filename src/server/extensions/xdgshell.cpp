/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/QWaylandPointer>
#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>

#include "xdgshell.h"
#include "xdgshell_p.h"
#include "serverlogging_p.h"

namespace GreenIsland {

namespace Server {

/*
 * XdgShellPrivate
 */

XdgShellPrivate::XdgShellPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::xdg_shell()
    , m_popupClient(Q_NULLPTR)
    , m_initialUp(false)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

void XdgShellPrivate::ping(XdgSurface *surface)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    uint32_t serial = surface->surface()->compositor()->nextSerial();
    ping(serial, surface);
}

void XdgShellPrivate::ping(uint32_t serial, XdgSurface *surface)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    m_pings.insert(serial, surface);
    send_ping(XdgSurfacePrivate::get(surface)->resource()->handle, serial);
}

void XdgShellPrivate::addPopup(XdgPopup *popup, QWaylandInputDevice *inputDevice)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    if (m_popups.contains(popup))
        return;

    if (m_popups.isEmpty()) {
        m_popupClient = XdgPopupPrivate::get(popup)->resource()->client();
        m_initialUp = !inputDevice->pointer()->isButtonPressed();
    }

    m_popups.append(popup);
}

void XdgShellPrivate::removePopup(XdgPopup *popup)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    m_popups.removeOne(popup);
}

void XdgShellPrivate::shell_destroy(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_Q(XdgShell);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    Q_ASSERT(compositor);

    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    Q_ASSERT(client);

    // Count how many XdgSurfaces are left
    int xdgSurfaces = 0;
    Q_FOREACH (QWaylandSurface *surface, compositor->surfacesForClient(client)) {
        XdgSurface *xdgSurface = XdgSurface::findIn(surface);
        if (xdgSurface)
            xdgSurfaces++;
    }

    // All xdg surfaces must be destroyed before xdg shell
    if (xdgSurfaces > 0)
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "Client must destroy surfaces before calling xdg_shell_destroy");
}

void XdgShellPrivate::shell_use_unstable_version(Resource *resource, int32_t version)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    if (version != version_current)
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "incompatible version, server is %d client wants %d",
                               version_current, version);
}

void XdgShellPrivate::shell_get_xdg_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_Q(XdgShell);

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    Q_ASSERT(compositor);

    // TODO: Implement the surface role pattern.
    if (0) {
        wl_resource_post_error(resource->handle, XdgShellPrivate::error_role,
                               "wl_surface@%d already has a role", id);
        return;
    }

    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    Q_ASSERT(client);

    Q_EMIT q->createSurface(surface, client, id);
}

void XdgShellPrivate::shell_get_xdg_popup(Resource *resource, uint32_t id, wl_resource *surfaceResource,
                                          wl_resource *parentResource, wl_resource *seatResource,
                                          uint32_t serial, int32_t x, int32_t y)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(serial);

    Q_Q(XdgShell);

    QWaylandSurface *parent = QWaylandSurface::fromResource(parentResource);
    if (!parent) {
        int id = wl_resource_get_id(surfaceResource);
        wl_resource_post_error(resource->handle, XdgPopupPrivate::error_invalid_parent,
                               "Invalid parent surface for popup wl_surface@%d", id);
        return;
    }

    if (!XdgSurface::findIn(parent) && !XdgPopup::findIn(parent)) {
        int id = wl_resource_get_id(surfaceResource);
        int parentId = wl_resource_get_id(parentResource);
        wl_resource_post_error(resource->handle, XdgPopupPrivate::error_invalid_parent,
                               "wl_surface@%d parent of wl_surface@%d is neither xdg_surface nor xdg_popup",
                               parentId, id);
        return;
    }

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    XdgPopup *popup = XdgPopup::findIn(surface);
    if (popup) {
        wl_resource_post_error(resource->handle, XdgShellPrivate::error_role,
                               "wl_surface@%d surface is already a xdg_popup", id);
        return;
    }

    QWaylandInputDevice *inputDevice = QWaylandInputDevice::fromSeatResource(seatResource);
    Q_ASSERT(inputDevice);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    Q_ASSERT(compositor);

    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    Q_ASSERT(client);

    Q_EMIT q->createPopup(inputDevice, surface, parent,
                          QPoint(x, y), client, id);
}

void XdgShellPrivate::shell_pong(Resource *resource, uint32_t serial)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    XdgSurfacePtr surface = m_pings.take(serial);
    if (!surface.isNull())
        Q_EMIT surface.data()->pong();
    else
        qCWarning(gLcXdgShell) << "Received an unexpected pong!";
}

/*
 * XdgShell
 */

XdgShell::XdgShell()
    : QWaylandExtensionTemplate<XdgShell>(*new XdgShellPrivate())
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

XdgShell::XdgShell(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<XdgShell>(compositor, *new XdgShellPrivate())
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

XdgShell::~XdgShell()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

void XdgShell::initialize()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_D(XdgShell);

    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing XdgShell";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *XdgShell::interface()
{
    return XdgShellPrivate::interface();
}

QByteArray XdgShell::interfaceName()
{
    return XdgShellPrivate::interfaceName();
}

#if 0
XdgPopupGrabber *XdgShell::popupGrabberForDevice(QtWayland::InputDevice *device)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    // Create popup grabbers on demand
    if (!m_popupGrabbers.contains(device))
        m_popupGrabbers.insert(device, new XdgPopupGrabber(device));
    return m_popupGrabbers.value(device);
}
#endif

/*
 * XdgSurfacePrivate
 */

XdgSurfacePrivate::XdgSurfacePrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::xdg_surface()
    , m_shell(Q_NULLPTR)
    , m_surface(Q_NULLPTR)
    , m_windowGeometry(QRect())
    , m_active(false)
    , m_transient(false)
    , m_maximized(false)
    , m_fullScreen(false)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

XdgSurfacePrivate::~XdgSurfacePrivate()
{
}

void XdgSurfacePrivate::sendActivate()
{
    PendingChange change;
    change.activated = m_active;
    sendConfigure(change);
}

void XdgSurfacePrivate::sendConfigure(const PendingChange &change)
{
    struct wl_array states;
    uint32_t *s;

    wl_array_init(&states);

    if (change.maximized) {
        s = (uint32_t *)wl_array_add(&states, sizeof *s);
        *s = XdgSurfacePrivate::state_maximized;
    } else if (change.fullScreen) {
        s = (uint32_t *)wl_array_add(&states, sizeof *s);
        *s = XdgSurfacePrivate::state_fullscreen;
    }

    if (change.resizing) {
        s = (uint32_t *)wl_array_add(&states, sizeof *s);
        *s = XdgSurfacePrivate::state_resizing;
    }

    if (change.activated) {
        s = (uint32_t *)wl_array_add(&states, sizeof *s);
        *s = XdgSurfacePrivate::state_activated;
    }

    uint32_t serial = m_surface->compositor()->nextSerial();
    m_pendingChanges[serial] = change;

    QByteArray statesArray((const char *)states.data, states.size);
    send_configure(change.size.width(), change.size.height(), statesArray, serial);

    wl_array_release(&states);
}

void XdgSurfacePrivate::surface_destroy_resource(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(XdgSurface);
    delete q;
}

void XdgSurfacePrivate::surface_destroy(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

void XdgSurfacePrivate::surface_set_parent(Resource *resource, wl_resource *parentResource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(XdgSurface);

    XdgSurface *parentSurface = Q_NULLPTR;
    if (parentResource)
        parentSurface = XdgSurface::fromResource(parentResource);

    if (parentSurface) {
        //XdgSurface *parentXdgSurface = XdgSurface::findIn(parentSurface);
        //if (parentXdgSurface)
        //XdgSurfacePrivate::get(parentXdgSurface)->m_hasChildren;
        // TODO:

        m_transient = true;
        Q_EMIT q->setTransient(parentSurface->surface());
    } else {
        m_transient = false;
        Q_EMIT q->setDefaultToplevel();
    }
}

void XdgSurfacePrivate::surface_set_title(Resource *resource, const QString &title)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    if (title == m_title)
        return;

    Q_Q(XdgSurface);
    m_title = title;
    Q_EMIT q->titleChanged();
}

void XdgSurfacePrivate::surface_set_app_id(Resource *resource, const QString &app_id)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    if (app_id == m_appId)
        return;

    Q_Q(XdgSurface);
    m_appId = app_id;
    Q_EMIT q->appIdChanged();
}

void XdgSurfacePrivate::surface_show_window_menu(Resource *resource, wl_resource *seat,
                                                 uint32_t serial, int32_t x, int32_t y)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(XdgSurface);

    QWaylandInputDevice *inputDevice = QWaylandInputDevice::fromSeatResource(seat);
    Q_EMIT q->windowMenuRequested(inputDevice, QPoint(x, y));
}

void XdgSurfacePrivate::surface_move(Resource *resource, wl_resource *seat, uint32_t serial)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(XdgSurface);

    QWaylandInputDevice *inputDevice = QWaylandInputDevice::fromSeatResource(seat);
    Q_EMIT q->startMove(inputDevice);
}

void XdgSurfacePrivate::surface_resize(Resource *resource, wl_resource *seat,
                                       uint32_t serial, uint32_t edges)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(XdgSurface);

    QWaylandInputDevice *inputDevice = QWaylandInputDevice::fromSeatResource(seat);
    Q_EMIT q->startResize(inputDevice, XdgSurface::ResizeEdge(edges));
}

void XdgSurfacePrivate::surface_ack_configure(Resource *resource, uint32_t serial)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(XdgSurface);

    // Can't continue if we don't have the pending change
    if (m_pendingChanges.find(serial) == m_pendingChanges.end())
        return;

    // Retrieve the pending change
    PendingChange change = m_pendingChanges.take(serial);

    // Set state
    if (change.maximized != m_maximized) {
        m_maximized = change.maximized;
        Q_EMIT q->maximizedChanged(change.output);
    }
    if (change.fullScreen != m_fullScreen) {
        m_fullScreen = change.fullScreen;
        Q_EMIT q->fullScreenChanged(change.output);
    }
    if (change.activated != m_active) {
        m_active = change.activated;
        Q_EMIT q->activeChanged();
    }
}

void XdgSurfacePrivate::surface_set_window_geometry(Resource *resource,
                                                    int32_t x, int32_t y,
                                                    int32_t width, int32_t height)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(XdgSurface);

    if (width > 0 && height > 0) {
        m_windowGeometry = QRect(QPoint(x, y), QSize(width, height));
        Q_EMIT q->windowGeometryChanged();
    }
}

void XdgSurfacePrivate::surface_set_maximized(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    // Only top level windows can be maximized
    if (m_transient)
        return;

    // Ignore if already maximized
    if (m_maximized)
        return;

    // Default output available geometry
    QWaylandOutput *output = m_surface->compositor()->defaultOutput();
    QRect geometry = output->availableGeometry();

    // Save size
    m_maximizedSize = m_surface->size();

    // Ask to resize the surface
    PendingChange change;
    change.maximized = true;
    change.fullScreen = m_fullScreen;
    change.resizing = true;
    change.activated = m_active;
    change.size = geometry.size();
    change.output = output;
    sendConfigure(change);
}

void XdgSurfacePrivate::surface_unset_maximized(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    // Restore the saved state
    PendingChange change;
    change.maximized = false;
    change.fullScreen = m_fullScreen;
    change.resizing = true;
    change.activated = m_active;
    change.size = m_maximizedSize;
    sendConfigure(change);
}

void XdgSurfacePrivate::surface_set_fullscreen(Resource *resource, wl_resource *outputResource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    // Only top level windows can be full screen
    if (m_transient)
        return;

    // Ignore if already full screen
    if (m_fullScreen)
        return;

    // Save size
    m_fullScreenSize = m_surface->size();

    // Determine the output
    QWaylandOutput *output = Q_NULLPTR;
    if (outputResource)
        output = QWaylandOutput::fromResource(outputResource);
    else
        output = m_surface->compositor()->defaultOutput();
    Q_ASSERT(output);

    // Ask to resize the surface
    PendingChange change;
    change.maximized = m_maximized;
    change.fullScreen = true;
    change.resizing = true;
    change.activated = m_active;
    change.size = output->geometry().size();
    change.output = output;
    sendConfigure(change);
}

void XdgSurfacePrivate::surface_unset_fullscreen(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    // Ask to resize the surface
    PendingChange change;
    change.maximized = m_maximized;
    change.fullScreen = false;
    change.resizing = true;
    change.activated = m_active;
    change.size = m_fullScreenSize;
    sendConfigure(change);
}

void XdgSurfacePrivate::surface_set_minimized(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(XdgSurface);
    Q_EMIT q->minimize();
}

/*
 * XdgSurface
 */

XdgSurface::XdgSurface()
    : QWaylandExtensionTemplate<XdgSurface>(*new XdgSurfacePrivate())
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

XdgSurface::XdgSurface(XdgShell *shell, QWaylandSurface *surface, QWaylandClient *client, uint id)
    : QWaylandExtensionTemplate<XdgSurface>(*new XdgSurfacePrivate())
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    initialize(shell, surface, client, id);
}

XdgSurface::~XdgSurface()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

void XdgSurface::initialize(XdgShell *shell, QWaylandSurface *surface, QWaylandClient *client, uint id)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_D(XdgSurface);
    d->m_shell = shell;
    d->m_surface = surface;
    d->m_windowGeometry = QRect(QPoint(0, 0), surface->size());
    d->init(client->client(), id, 1);
    setExtensionContainer(surface);
    Q_EMIT surfaceChanged();
    QWaylandExtension::initialize();
}

void XdgSurface::initialize()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    QWaylandExtensionTemplate::initialize();
}

QWaylandSurface *XdgSurface::surface() const
{
    Q_D(const XdgSurface);
    return d->m_surface;
}

QString XdgSurface::title() const
{
    Q_D(const XdgSurface);
    return d->m_title;
}

QString XdgSurface::appId() const
{
    Q_D(const XdgSurface);
    return d->m_appId;
}

QRect XdgSurface::windowGeometry() const
{
    Q_D(const XdgSurface);
    return d->m_windowGeometry;
}

bool XdgSurface::isActive() const
{
    Q_D(const XdgSurface);
    return d->m_active;
}

void XdgSurface::setActive(bool active)
{
    Q_D(XdgSurface);

    if (d->m_active == active)
        return;

    d->m_active = active;
    Q_EMIT activeChanged();

    // Tell the client
    d->sendActivate();
}

bool XdgSurface::isMaximized() const
{
    Q_D(const XdgSurface);
    return d->m_maximized;
}

bool XdgSurface::isFullScreen() const
{
    Q_D(const XdgSurface);
    return d->m_fullScreen;
}

void XdgSurface::ping()
{
    // TODO:
    return;
    Q_D(XdgSurface);
    XdgShellPrivate::get(d->m_shell)->ping(this);
    Q_EMIT pingRequested();
}

QSize XdgSurface::sizeForResize(const QSizeF &size, const QPointF &delta, ResizeEdge edge)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    qreal width = size.width();
    qreal height = size.height();
    if (edge & LeftEdge)
        width -= delta.x();
    else if (edge & RightEdge)
        width += delta.x();

    if (edge & TopEdge)
        height -= delta.y();
    else if (edge & BottomEdge)
        height += delta.y();

    return QSizeF(width, height).toSize();
}

void XdgSurface::sendConfigure(const QSize &size)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_D(XdgSurface);

    XdgSurfacePrivate::PendingChange change;
    change.maximized = d->m_maximized;
    change.fullScreen = d->m_fullScreen;
    change.resizing = true;
    change.activated = d->m_active;
    change.size = size;
    d->sendConfigure(change);
}

void XdgSurface::close()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_D(XdgSurface);
    d->send_close();
}

XdgSurface *XdgSurface::fromResource(wl_resource *res)
{
    return static_cast<XdgSurfacePrivate *>(XdgSurfacePrivate::Resource::fromResource(res)->surface_object)->q_func();
}

const struct wl_interface *XdgSurface::interface()
{
    return XdgSurfacePrivate::interface();
}

QByteArray XdgSurface::interfaceName()
{
    return XdgSurfacePrivate::interfaceName();
}

/*
 * XdgPopupPrivate
 */

XdgPopupPrivate::XdgPopupPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::xdg_popup()
    , m_shell(Q_NULLPTR)
    , m_surface(Q_NULLPTR)
    , m_parentSurface(Q_NULLPTR)
    , m_inputDevice(Q_NULLPTR)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

XdgPopupPrivate::~XdgPopupPrivate()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

void XdgPopupPrivate::popup_destroy_resource(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_UNUSED(resource);

    Q_Q(XdgPopup);
    delete q;
}

void XdgPopupPrivate::popup_destroy(Resource *resource)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

/*
 * XdgPopup
 */

XdgPopup::XdgPopup()
    : QWaylandExtensionTemplate<XdgPopup>(*new XdgPopupPrivate())
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

XdgPopup::XdgPopup(XdgShell *shell, QWaylandInputDevice *inputDevice,
                   QWaylandSurface *surface, QWaylandSurface *parentSurface,
                   QWaylandClient *client, uint id)
    : QWaylandExtensionTemplate<XdgPopup>(*new XdgPopupPrivate())
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    initialize(shell, inputDevice, surface, parentSurface, client, id);
}

XdgPopup::~XdgPopup()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;
}

void XdgPopup::initialize(XdgShell *shell, QWaylandInputDevice *inputDevice,
                          QWaylandSurface *surface, QWaylandSurface *parentSurface,
                          QWaylandClient *client, uint id)
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_D(XdgPopup);
    d->m_shell = shell;
    d->m_inputDevice = inputDevice;
    d->m_surface = surface;
    d->m_parentSurface = parentSurface;
    d->init(client->client(), id, 1);
    setExtensionContainer(surface);
    Q_EMIT surfaceChanged();
    Q_EMIT parentSurfaceChanged();
    QWaylandExtension::initialize();

    XdgShellPrivate::get(shell)->addPopup(this, inputDevice);
}

void XdgPopup::initialize()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    QWaylandExtensionTemplate::initialize();
}

QWaylandSurface *XdgPopup::surface() const
{
    Q_D(const XdgPopup);
    return d->m_surface;
}

QWaylandSurface *XdgPopup::parentSurface() const
{
    Q_D(const XdgPopup);
    return d->m_parentSurface;
}

void XdgPopup::sendPopupDone()
{
    qCDebug(gLcXdgShellTrace) << Q_FUNC_INFO;

    Q_D(XdgPopup);
    d->send_popup_done();
}

const struct wl_interface *XdgPopup::interface()
{
    return XdgPopupPrivate::interface();
}

QByteArray XdgPopup::interfaceName()
{
    return XdgPopupPrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_xdgshell.cpp"
