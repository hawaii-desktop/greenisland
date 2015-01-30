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

#ifndef XDGSURFACE_H
#define XDGSURFACE_H

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceInterface>

#include "xdgshell.h"

class QQuickItem;
class QWaylandInputDevice;

namespace GreenIsland {

class ClientWindow;
class XdgSurfaceMoveGrabber;
class XdgSurfaceResizeGrabber;

class XdgSurface : public QObject, public QWaylandSurfaceInterface, public QtWaylandServer::xdg_surface
{
    Q_OBJECT
    Q_ENUMS(State)
public:
    enum State {
        Normal = 0,
        Maximized,
        FullScreen
    };

    struct Changes {
        Changes()
            : newState(false)
            , moving(false)
            , resizing(false)
            , active(false)
            , state(Normal)
            , position(QPointF())
            , size(QSizeF())
        {
        }

        bool newState;
        bool moving;
        bool resizing;

        bool active;
        State state;

        QPointF position;
        QSizeF size;
    };

    XdgSurface(XdgShell *shell, QWaylandSurface *surface,
               wl_client *client, uint32_t id);
    ~XdgSurface();

    uint32_t nextSerial() const;

    QWaylandSurface::WindowType type() const;

    State state() const;

    QWaylandSurface *surface() const;

    ClientWindow *window() const;

    void restore();
    void restoreAt(const QPointF &pos);

    void resetMoveGrab();
    void resetResizeGrab();

    void requestConfigure(const Changes &changes);

protected:
    bool runOperation(QWaylandSurfaceOp *op) Q_DECL_OVERRIDE;

private:
    XdgShell *m_shell;
    QWaylandSurface *m_surface;
    ClientWindow *m_window;

    XdgSurfaceMoveGrabber *m_moveGrabber;
    XdgSurfaceResizeGrabber *m_resizeGrabber;

    bool m_minimized;
    State m_state;

    State m_savedState;
    QRectF m_savedGeometry;

    QMap<uint32_t, Changes> m_pendingChanges;

    bool m_deleting;


    void moveWindow(QWaylandInputDevice *device);


    void surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void surface_destroy(Resource *resource) Q_DECL_OVERRIDE;

    void surface_set_parent(Resource *resource,
                            wl_resource *parentResource) Q_DECL_OVERRIDE;

    void surface_set_title(Resource *resource,
                           const QString &title) Q_DECL_OVERRIDE;
    void surface_set_app_id(Resource *resource,
                            const QString &app_id) Q_DECL_OVERRIDE;

    void surface_show_window_menu(Resource *resource, wl_resource *seat,
                                  uint32_t serial, int32_t x, int32_t y) Q_DECL_OVERRIDE;

    void surface_move(Resource *resource, wl_resource *seat,
                      uint32_t serial) Q_DECL_OVERRIDE;
    void surface_resize(Resource *resource, wl_resource *seat, uint32_t serial,
                        uint32_t edges) Q_DECL_OVERRIDE;

    void surface_ack_configure(Resource *resource,
                               uint32_t serial) Q_DECL_OVERRIDE;
    void surface_set_window_geometry(Resource *resource, int32_t x, int32_t y,
                                     int32_t width, int32_t height) Q_DECL_OVERRIDE;

    void surface_set_maximized(Resource *resource) Q_DECL_OVERRIDE;
    void surface_unset_maximized(Resource *resource) Q_DECL_OVERRIDE;

    void surface_set_fullscreen(Resource *resource,
                                wl_resource *outputResource) Q_DECL_OVERRIDE;
    void surface_unset_fullscreen(Resource *resource) Q_DECL_OVERRIDE;

    void surface_set_minimized(Resource *resource) Q_DECL_OVERRIDE;
};

}

#endif // XDGSURFACE_H
