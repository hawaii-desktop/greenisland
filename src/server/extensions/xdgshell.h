/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_XDGSHELL_H
#define GREENISLAND_XDGSHELL_H

#include <QtCore/QHash>

#include <GreenIsland/server/greenislandserver_export.h>
#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

class QWaylandClient;
class QWaylandInputDevice;
class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class XdgShellPrivate;
class XdgSurfacePrivate;
class XdgPopupPrivate;

class GREENISLANDSERVER_EXPORT XdgShell : public QWaylandExtensionTemplate<XdgShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(XdgShell)
public:
    XdgShell();
    XdgShell(QWaylandCompositor *compositor);
    ~XdgShell();

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void createSurface(QWaylandSurface *surface,
                       QWaylandClient *client, uint id);
    void createPopup(QWaylandInputDevice *inputDevice,
                     QWaylandSurface *surface,
                     QWaylandSurface *parentSurface,
                     const QPoint &relativeToParent,
                     QWaylandClient *client, uint id);
};

class GREENISLANDSERVER_EXPORT XdgSurface : public QWaylandExtensionTemplate<XdgSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(XdgSurface)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullScreen READ isFullScreen NOTIFY fullScreenChanged)
public:
    enum ResizeEdge {
        DefaultEdge     =  0,
        TopEdge         =  1,
        BottomEdge      =  2,
        LeftEdge        =  4,
        TopLeftEdge     =  5,
        BottomLeftEdge  =  6,
        RightEdge       =  8,
        TopRightEdge    =  9,
        BottomRightEdge = 10
    };
    Q_ENUM(ResizeEdge)

    enum State {
        Maximized   = 1,
        FullScreen  = 2,
        Resizing    = 3,
        Activated   = 4
    };
    Q_ENUM(State)

    XdgSurface();
    XdgSurface(XdgShell *shell, QWaylandSurface *surface,
               QWaylandClient *client, uint id);
    ~XdgSurface();

    Q_INVOKABLE void initialize(XdgShell *shell, QWaylandSurface *surface,
                                QWaylandClient *client, uint id);

    QWaylandSurface *surface() const;

    QString title() const;
    QString appId() const;

    QRect windowGeometry() const;

    bool isActive() const;
    bool isMaximized() const;
    bool isFullScreen() const;

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size,
                                    const QPointF &delta, ResizeEdge edge);
    Q_INVOKABLE void sendConfigure(const QSize &size);

    Q_INVOKABLE void close();

    static XdgSurface *fromResource(wl_resource *res);

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    void ping();
    void setActive(bool active);

Q_SIGNALS:
    void surfaceChanged();
    void titleChanged();
    void appIdChanged();
    void windowGeometryChanged();
    void activeChanged();
    void maximizedChanged(QWaylandOutput *output);
    void fullScreenChanged(QWaylandOutput *output);
    void minimize();

    void pingRequested();
    void pong();

    void setDefaultToplevel();
    void setTransient(QWaylandSurface *parentSurface);

    void windowMenuRequested(QWaylandInputDevice *inputDevice,
                             const QPoint &position);

    void startMove(QWaylandInputDevice *inputDevice);
    void startResize(QWaylandInputDevice *inputDevice, ResizeEdge edges);

private:
    void initialize();
};

class GREENISLANDSERVER_EXPORT XdgPopup : public QWaylandExtensionTemplate<XdgPopup>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(XdgPopup)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandSurface *parentSurface READ parentSurface NOTIFY parentSurfaceChanged)
public:
    XdgPopup();
    XdgPopup(XdgShell *shell, QWaylandInputDevice *inputDevice,
             QWaylandSurface *surface, QWaylandSurface *parentSurface,
             QWaylandClient *client, uint id);
    ~XdgPopup();

    Q_INVOKABLE void initialize(XdgShell *shell,
                                QWaylandInputDevice *inputDevice,
                                QWaylandSurface *surface, QWaylandSurface *parentSurface,
                                QWaylandClient *client, uint id);

    QWaylandSurface *surface() const;
    QWaylandSurface *parentSurface() const;

    Q_INVOKABLE void sendPopupDone();

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void surfaceChanged();
    void parentSurfaceChanged();

private:
    void initialize();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_XDGSHELL_H
