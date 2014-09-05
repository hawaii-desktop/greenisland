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

#ifndef WLSHELLSURFACE_H
#define WLSHELLSURFACE_H

#include <QtCore/QSet>
#include <QtQuick/QQuickItem>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceInterface>

#include "wlshell.h"

class QWaylandInputDevice;

class WindowView;
class WlShellSurfaceMoveGrabber;
class WlShellSurfaceResizeGrabber;
class WlShellSurfacePopupGrabber;

class WlShellSurface : public QObject, public QWaylandSurfaceInterface, public QtWaylandServer::wl_shell_surface
{
    Q_OBJECT
    Q_ENUMS(State)
public:
    enum State {
        Normal = 0,
        Maximized,
        FullScreen
    };

    enum Operation {
        Move = QWaylandSurfaceOp::UserType
    };

    explicit WlShellSurface(WlShell *shell, QWaylandSurface *surface,
                            wl_client *client, uint32_t id);
    ~WlShellSurface();

    State state() const;
    void setState(const State &state);

    WindowView *view() const;

    QQuickItem *window() const;
    QQuickItem *transientParent() const;

    void setPosition(const QPointF &pt);
    void setGeometry(const QRectF &geometry);

    void setOffset(const QPointF &pt);

    void restore();

protected:
    bool runOperation(QWaylandSurfaceOp *op) Q_DECL_OVERRIDE;

private:
    WlShell *m_shell;
    QWaylandSurface *m_surface;
    WindowView *m_view;

    WlShellSurfaceMoveGrabber *m_moveGrabber;
    WlShellSurfaceResizeGrabber *m_resizeGrabber;
    WlShellSurfacePopupGrabber *m_popupGrabber;

    QSet<uint32_t> m_pings;

    State m_state;
    State m_prevState;
    QRectF m_prevGlobalGeometry;

    bool m_deleting;

    void ping(uint32_t serial);
    void moveWindow(QWaylandInputDevice *device);
    void requestResize(const QSize &size);


    void shell_surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void shell_surface_pong(Resource *resource, uint32_t serial) Q_DECL_OVERRIDE;
    void shell_surface_move(Resource *resource, wl_resource *seat, uint32_t serial) Q_DECL_OVERRIDE;
    void shell_surface_resize(Resource *resource, wl_resource *seat, uint32_t serial, uint32_t edges) Q_DECL_OVERRIDE;

    void shell_surface_set_toplevel(Resource *resource) Q_DECL_OVERRIDE;
    void shell_surface_set_transient(Resource *resource,
                                     wl_resource *parentResource,
                                     int32_t x, int32_t y,
                                     uint32_t flags) Q_DECL_OVERRIDE;
    void shell_surface_set_fullscreen(Resource *resource, uint32_t method,
                                      uint32_t framerate,
                                      wl_resource *outputResource) Q_DECL_OVERRIDE;
    void shell_surface_set_popup(Resource *resource, wl_resource *seat,
                                 uint32_t serial, wl_resource *parent,
                                 int32_t x, int32_t y, uint32_t flags) Q_DECL_OVERRIDE;
    void shell_surface_set_maximized(Resource *resource,
                                     wl_resource *outputResource) Q_DECL_OVERRIDE;

    void shell_surface_set_title(Resource *resource,
                                 const QString &title) Q_DECL_OVERRIDE;
    void shell_surface_set_class(Resource *resource,
                                 const QString &class_) Q_DECL_OVERRIDE;

    friend class WlShellSurfaceMoveGrabber;
    friend class WlShellSurfaceResizeGrabber;
};

#endif // WLSHELLSURFACE_H
