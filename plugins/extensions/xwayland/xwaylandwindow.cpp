/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

/*
 * This code takes inspiration from the xwayland module from Weston,
 * which is:
 *
 * Copyright (C) 2011 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <GreenIsland/Compositor/QWaylandCompositor>
#include <GreenIsland/Compositor/QWaylandSurfaceView>

#include <GreenIsland/Server/ClientWindow>
#include <GreenIsland/Server/WindowView>

#include "xcbwrapper.h"
#include "xcbresources.h"
#include "xwayland.h"
#include "xwaylandmanager.h"
#include "xwaylandwindow.h"
#include "xwaylandsurface.h"

#define TYPE_WM_PROTOCOLS       XCB_ATOM_CUT_BUFFER0
#define TYPE_MOTIF_WM_HINTS     XCB_ATOM_CUT_BUFFER1
#define TYPE_NET_WM_STATE       XCB_ATOM_CUT_BUFFER2
#define TYPE_WM_NORMAL_HINTS    XCB_ATOM_CUT_BUFFER3

namespace GreenIsland {

XWaylandWindow::XWaylandWindow(xcb_window_t window, const QRect &geometry,
                               bool overrideRedirect, XWaylandManager *parent)
    : QObject(parent)
    , m_wm(parent)
    , m_window(window)
    , m_geometry(geometry)
    , m_propsDirty(true)
    , m_overrideRedirect(overrideRedirect)
    , m_decorated(false)
    , m_transientFor(Q_NULLPTR)
    , m_surfaceId(0)
    , m_surface(Q_NULLPTR)
    , m_surfaceInterface(Q_NULLPTR)
    , m_wmState(WithdrawnState)
    , m_workspace(0)
{
    m_properties.pos = geometry.topLeft();
    m_properties.size = geometry.size();
    m_properties.fullscreen = 0;
    m_properties.maximizedHorizontally = 0;
    m_properties.maximizedVertically = 0;
    m_properties.deleteWindow = 0;

    xcb_get_geometry_cookie_t cookie =
            xcb_get_geometry(Xcb::connection(), window);

    quint32 values[1];
    values[0] = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(Xcb::connection(), window,
                                 XCB_CW_EVENT_MASK, values);

    xcb_get_geometry_reply_t *reply =
            xcb_get_geometry_reply(Xcb::connection(), cookie, Q_NULLPTR);
    if (reply)
        m_hasAlpha = reply->depth == 32;
    else
        m_hasAlpha = false;
    free(reply);

    m_wm->addWindow(window, this);
}

XWaylandWindow::~XWaylandWindow()
{
#if 0
    if (m_frameId) {
        xcb_reparent_window(Xcb::connection(), m_window, wm_window, 0, 0);
        xcb_destroy_window(Xcb::connection(), m_frameId);
        setWmState(ICCCM_WITHDRAWN_STATE);
        m_wm->removeWindow(m_frameId);
        m_frameId = XCB_WINDOW_NONE;
    }
#endif

    setSurface(Q_NULLPTR);

    m_wm->removeWindow(m_window);
}

void XWaylandWindow::setSurface(QWaylandSurface *surface)
{
    // A XWaylandWindow may have different surfaces assigned
    // during its life, disconnect everything before assigning it
    if (m_surface) {
        disconnect(m_surface.data(), &QWaylandSurface::surfaceDestroyed,
                   this, &XWaylandWindow::surfaceDestroyed);
        if (m_surfaceInterface) {
            m_surface.data()->removeInterface(m_surfaceInterface);
            m_surfaceInterface->deleteLater();
            m_surfaceInterface = Q_NULLPTR;
        }
    }

    // A null surface means unmap
    if (!surface) {
        if (m_surfaceInterface) {
            if (m_surface)
                m_surface.data()->removeInterface(m_surfaceInterface);
            m_surfaceInterface->deleteLater();
        }
        m_surface = Q_NULLPTR;
        m_surfaceInterface = Q_NULLPTR;
        return;
    }

    // Connect the new surface and create the interface
    m_surface = surface;
    connect(m_surface.data(), &QWaylandSurface::surfaceDestroyed,
            this, &XWaylandWindow::surfaceDestroyed);
    if (m_surfaceInterface)
        m_surfaceInterface->deleteLater();
    m_surfaceInterface = new XWaylandSurface(this);

    // Read and set properties
    readProperties();
    setProperties();

    // Move the window
    m_surfaceInterface->clientWindow()->setPosition(m_properties.pos);
}

ClientWindow *XWaylandWindow::clientWindow() const
{
    if (m_surfaceInterface)
        return m_surfaceInterface->clientWindow();
    return Q_NULLPTR;
}

void XWaylandWindow::setWmState(WmState state)
{
    quint32 property[2];
    property[0] = state;
    property[1] = XCB_WINDOW_NONE;

    xcb_change_property(Xcb::connection(),
                        XCB_PROP_MODE_REPLACE,
                        m_window,
                        Xcb::resources()->atoms->wm_state,
                        Xcb::resources()->atoms->wm_state,
                        32, 2, property);
}

void XWaylandWindow::setNetWmState()
{
    quint32 property[3];
    quint32 i = 0;

    if (m_properties.fullscreen)
        property[i++] = Xcb::resources()->atoms->net_wm_state_fullscreen;
    if (m_properties.maximizedHorizontally)
        property[i++] = Xcb::resources()->atoms->net_wm_state_maximized_horz;
    if (m_properties.maximizedVertically)
        property[i++] = Xcb::resources()->atoms->net_wm_state_maximized_vert;

    xcb_change_property(Xcb::connection(),
                        XCB_PROP_MODE_REPLACE,
                        m_window,
                        Xcb::resources()->atoms->net_wm_state,
                        XCB_ATOM_ATOM,
                        32, i, property);
}

void XWaylandWindow::setWorkspace(int workspace)
{
    // Passing a workspace < 0 deletes the property
    if (workspace >= 0) {
        xcb_change_property(Xcb::connection(),
                            XCB_PROP_MODE_REPLACE,
                            m_window,
                            Xcb::resources()->atoms->net_wm_desktop,
                            XCB_ATOM_CARDINAL,
                            31, 1, &workspace);
    } else {
        xcb_delete_property(Xcb::connection(),
                            m_window, Xcb::resources()->atoms->net_wm_desktop);
    }
}

void XWaylandWindow::readProperties()
{
    if (!m_propsDirty)
        return;
    m_propsDirty = false;

    QMap<xcb_atom_t, xcb_atom_t> props;
    props[XCB_ATOM_WM_CLASS] = XCB_ATOM_STRING;
    props[XCB_ATOM_WM_NAME] = XCB_ATOM_STRING;
    props[XCB_ATOM_WM_TRANSIENT_FOR] = XCB_ATOM_WINDOW;
    props[Xcb::resources()->atoms->wm_protocols] = TYPE_WM_PROTOCOLS;
    props[Xcb::resources()->atoms->wm_normal_hints] = TYPE_WM_NORMAL_HINTS;
    props[Xcb::resources()->atoms->net_wm_state] = TYPE_NET_WM_STATE;
    props[Xcb::resources()->atoms->net_wm_window_type] = XCB_ATOM_ATOM;
    props[Xcb::resources()->atoms->net_wm_name] = XCB_ATOM_STRING;
    props[Xcb::resources()->atoms->net_wm_pid] = XCB_ATOM_CARDINAL;
    props[Xcb::resources()->atoms->motif_wm_hints] = TYPE_MOTIF_WM_HINTS;
    props[Xcb::resources()->atoms->wm_client_machine] = XCB_ATOM_WM_CLIENT_MACHINE;

    QMap<xcb_atom_t, xcb_get_property_cookie_t> cookies;
    Q_FOREACH (xcb_atom_t atom, props.keys()) {
        xcb_get_property_cookie_t cookie = xcb_get_property(
                    Xcb::connection(), 0, m_window, atom, XCB_ATOM_ANY, 0, 2048);
        cookies[atom] = cookie;
    }

    m_decorated = !m_overrideRedirect;
    m_sizeHints.flags = 0;
    m_motifHints.flags = 0;
    m_properties.deleteWindow = 0;

    Q_FOREACH (xcb_atom_t atom, props.keys()) {
        xcb_get_property_reply_t *reply =
                xcb_get_property_reply(Xcb::connection(), cookies[atom], Q_NULLPTR);
        if (!reply)
            // Bad window, usually
            continue;
        if (reply->type == XCB_ATOM_NONE) {
            // No such property
            free(reply);
            continue;
        }

        void *p = decodeProperty(props[atom], reply);
        dumpProperty(atom, reply);

        switch (atom) {
        case XCB_ATOM_WM_CLASS:
            m_properties.appId = QString::fromUtf8((char *)p);
            free(p);
            break;
        case XCB_ATOM_WM_NAME:
            m_properties.title = QString::fromUtf8((char *)p);
            free(p);
            break;
        case XCB_ATOM_WM_TRANSIENT_FOR:
            m_transientFor = (XWaylandWindow *)p;
            break;
        default:
            break;
        }

        free(reply);
    }

    setProperties();
}

void XWaylandWindow::setProperties()
{
    if (!m_surfaceInterface)
        return;

    m_surfaceInterface->setAppId(m_properties.appId);
    m_surfaceInterface->setTitle(m_properties.title);
}

void XWaylandWindow::readAndDumpProperty(xcb_atom_t atom)
{
    xcb_get_property_cookie_t cookie =
            xcb_get_property(Xcb::connection(), 0, m_window,
                             atom, XCB_ATOM_ANY, 0, 2048);
    xcb_get_property_reply_t *reply =
            xcb_get_property_reply(Xcb::connection(), cookie, Q_NULLPTR);

    dumpProperty(atom, reply);

    free(reply);
}

void XWaylandWindow::requestResize(const QSize &size)
{
    m_properties.size = size;

    if (!m_surface)
        return;

    m_surface->requestSize(size);
    Q_FOREACH (QWaylandSurfaceView *surfView, m_surface->views()) {
        WindowView *view = static_cast<WindowView *>(surfView);
        if (view) {
            view->setResizeSurfaceToItem(true);
            view->setSize(size);
        }
    }
}

void XWaylandWindow::resizeFrame(const QSize &size, quint32 mask, quint32 *values)
{
    // Resize surface
    requestResize(size);
    /*
    if (window->frame)
                    frame_resize_inside(window->frame, window->width, window->height);
*/



    xcb_configure_window(Xcb::connection(), m_window, mask, values);

    //weston_wm_window_get_frame_size(window, &width, &height);
    m_frameWindow.resize(size.width() + 10, size.height() + 10);
}

void XWaylandWindow::map()
{
    xcb_map_window(Xcb::connection(), m_window);
    m_frameWindow.map();
}

void XWaylandWindow::unmap()
{
    m_frameWindow.unmap();
}

void XWaylandWindow::repaint()
{
    if (!m_surface || !m_surface->compositor())
        return;
    m_surface->compositor()->sendFrameCallbacks(QList<QWaylandSurface *>() << m_surface);
}

XWaylandWindow::operator xcb_window_t() const
{
    return m_window;
}

xcb_window_t XWaylandWindow::window() const
{
    return m_window;
}

void XWaylandWindow::dumpProperty(xcb_atom_t property, xcb_get_property_reply_t *reply)
{
    QString buffer = QString("\tProperty %1 (window %2): ")
            .arg(Xcb::Atom::nameFromAtom(property))
            .arg(m_window);

    if (!reply)
        buffer += QStringLiteral("(no reply)");

    qCDebug(XWAYLAND_TRACE) << qPrintable(buffer);

    if (!reply)
        return;

    buffer = buffer.sprintf("\t\t%s/%d, length %d (value_len %d): ",
                            qPrintable(Xcb::Atom::nameFromAtom(reply->type)),
                            reply->format,
                            xcb_get_property_value_length(reply),
                            reply->value_len);

    if (reply->type == Xcb::resources()->atoms->incr) {
        qint32 *v = (qint32 *)xcb_get_property_value(reply);
        buffer += buffer.sprintf("%d", *v);
    } else if (reply->type == Xcb::resources()->atoms->utf8_string ||
               reply->type == Xcb::resources()->atoms->string) {
        const char *v = (const char *)xcb_get_property_value(reply);
        int len = reply->value_len > 40 ? 40 : reply->value_len;
        buffer += buffer.sprintf("\"%.*s\"\n", len, v);
    } else if (reply->type == XCB_ATOM_ATOM) {
        xcb_atom_t *v = (xcb_atom_t *)xcb_get_property_value(reply);

        for (quint32 i = 0; i < reply->value_len; i++) {
            QString name = Xcb::Atom::nameFromAtom(v[i]);
            if (i > 0)
                buffer += buffer.sprintf(", %s", qPrintable(name));
            else
                buffer += name;
        }
    } else {
        buffer += QStringLiteral("huh?");
    }

    qCDebug(XWAYLAND_TRACE) << qPrintable(buffer);
}

void *XWaylandWindow::decodeProperty(xcb_atom_t type, xcb_get_property_reply_t *reply)
{
    switch (type) {
    case XCB_ATOM_WM_CLIENT_MACHINE:
    case XCB_ATOM_STRING: {
        char *p = strndup((char *)xcb_get_property_value(reply),
                          xcb_get_property_value_length(reply));
        return p;
    }
    case XCB_ATOM_WINDOW: {
        xcb_window_t *xid = (xcb_window_t *)xcb_get_property_value(reply);
        return m_wm->windowFromId(*xid);
    }
    case XCB_ATOM_CARDINAL:
    case XCB_ATOM_ATOM:
        return xcb_get_property_value(reply);
    case TYPE_WM_PROTOCOLS: {
        xcb_atom_t *value = (xcb_atom_t *)xcb_get_property_value(reply);
        for (uint32_t i = 0; i < reply->value_len; i++)
            if (value[i] == Xcb::resources()->atoms->wm_delete_window)
                m_properties.deleteWindow = 1;
        break;
    }
    case TYPE_WM_NORMAL_HINTS:
        memcpy(&m_sizeHints,
               xcb_get_property_value(reply),
               sizeof m_sizeHints);
        break;
    case TYPE_NET_WM_STATE: {
        m_properties.fullscreen = 0;
        xcb_atom_t *value = (xcb_atom_t *)xcb_get_property_value(reply);
        uint32_t i;
        for (i = 0; i < reply->value_len; i++)
            if (value[i] == Xcb::resources()->atoms->net_wm_state_fullscreen)
                m_properties.fullscreen = 1;
        if (value[i] == Xcb::resources()->atoms->net_wm_state_maximized_horz)
            m_properties.maximizedHorizontally = 1;
        if (value[i] == Xcb::resources()->atoms->net_wm_state_maximized_vert)
            m_properties.maximizedVertically = 1;
        break;
    }
    case TYPE_MOTIF_WM_HINTS:
        memcpy(&m_motifHints,
               xcb_get_property_value(reply),
               sizeof m_motifHints);
        if (m_motifHints.flags & MWM_HINTS_DECORATIONS)
            m_decorated = m_motifHints.decorations;
        break;
    default:
        break;
    }

    return Q_NULLPTR;
}

void XWaylandWindow::surfaceDestroyed()
{
}

}

#include "moc_xwaylandwindow.cpp"
