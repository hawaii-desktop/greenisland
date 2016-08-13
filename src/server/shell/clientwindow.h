/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

#ifndef GREENISLAND_CLIENTWINDOW_H
#define GREENISLAND_CLIENTWINDOW_H

#include <QtCore/QObject>

#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>
#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>

#include <GreenIsland/server/greenislandserver_export.h>

class QWaylandWlShellSurface;
class QWaylandXdgSurface;
class QWaylandXdgPopup;

namespace GreenIsland {

namespace Server {

class ApplicationManager;
class ClientWindowPrivate;
class GtkSurface;

class GREENISLANDSERVER_EXPORT ClientWindow : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClientWindow)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(GreenIsland::Server::ClientWindow *parentWindow READ parentWindow NOTIFY parentWindowChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(qint64 processId READ processId NOTIFY processIdChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(qreal x READ x NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y NOTIFY yChanged)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)
    Q_PROPERTY(QRect taskIconGeometry READ taskIconGeometry WRITE setTaskIconGeometry NOTIFY taskIconGeometryChanged)
    Q_PROPERTY(bool activated READ activated NOTIFY activatedChanged)
    Q_PROPERTY(bool minimized READ minimized WRITE setMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem CONSTANT)
    Q_PROPERTY(QWaylandOutput *designedOutput READ designedOutput NOTIFY designedOutputChanged)
    Q_PRIVATE_PROPERTY(ClientWindow::d_func(), QQmlListProperty<QWaylandQuickItem> views READ windowViews NOTIFY viewsChanged DESIGNABLE false)
    Q_PRIVATE_PROPERTY(ClientWindow::d_func(), QQmlListProperty<QWaylandOutput> outputs READ outputs NOTIFY outputsChanged DESIGNABLE false)
public:
    enum Type {
        Unknown = 0,
        TopLevel,
        Transient,
        Popup
    };
    Q_ENUM(Type)

    QWaylandSurface *surface() const;

    Type type() const;

    ClientWindow *parentWindow() const;

    QString title() const;
    QString appId() const;
    qint64 processId() const;
    QString iconName() const;

    qreal x() const;
    qreal y() const;

    QPointF position() const;

    QRect windowGeometry() const;

    QRect taskIconGeometry() const;
    void setTaskIconGeometry(const QRect &rect);

    bool activated() const;

    bool minimized() const;
    void setMinimized(bool minimized);

    bool maximized() const;
    bool fullscreen() const;

    QQuickItem *moveItem() const;

    QWaylandOutput *designedOutput() const;

    Q_INVOKABLE QPointF randomPosition() const;

    Q_INVOKABLE void addWindowView(QWaylandQuickItem *item);
    Q_INVOKABLE void removeWindowView(QWaylandQuickItem *item);

public Q_SLOTS:
    void activate();
    void lower();
    void raise();
    void move(const QPoint &globalPosition);
    void close();

Q_SIGNALS:
    void surfaceChanged();
    void processIdChanged();
    void typeChanged();
    void parentWindowChanged();
    void titleChanged();
    void appIdChanged();
    void iconNameChanged();
    void xChanged();
    void yChanged();
    void windowGeometryChanged();
    void taskIconGeometryChanged();
    void activatedChanged();
    void minimizedChanged();
    void maximizedChanged();
    void fullscreenChanged();
    void designedOutputChanged();
    void viewsChanged();
    void outputsChanged();
    void pingRequested();
    void pong();
    void showWindowMenu(QWaylandSeat *seat,
                        const QPoint &localSurfacePosition);

private:
    ClientWindow(ApplicationManager *applicationManager, QWaylandSurface *surface);

    Q_PRIVATE_SLOT(d_func(), void _q_wlShellSurfaceCreated(QWaylandWlShellSurface *wlShellSurface))
    Q_PRIVATE_SLOT(d_func(), void _q_xdgSurfaceCreated(QWaylandXdgSurface *xdgSurface))
    Q_PRIVATE_SLOT(d_func(), void _q_xdgPopupCreated(QWaylandXdgPopup *xdgPopup))
    Q_PRIVATE_SLOT(d_func(), void _q_gtkSurfaceCreated(GtkSurface *gtkSurface))
    Q_PRIVATE_SLOT(d_func(), void _q_handleDefaultSeatChanged(QWaylandSeat *newSeat, QWaylandSeat *oldSeat))
    Q_PRIVATE_SLOT(d_func(), void _q_handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface))

    friend class ApplicationManager;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_CLIENTWINDOW_H
