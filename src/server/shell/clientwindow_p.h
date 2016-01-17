/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

#ifndef GREENISLAND_CLIENTWINDOW_P_H
#define GREENISLAND_CLIENTWINDOW_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Server/ClientWindow>

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class WindowManager;

class GREENISLANDSERVER_EXPORT ClientWindowPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(ClientWindow)
public:
    ClientWindowPrivate()
        : wm(Q_NULLPTR)
        , surface(Q_NULLPTR)
        , type(ClientWindow::Unknown)
        , parentWindow(Q_NULLPTR)
        , pid(0)
        , x(0)
        , y(0)
        , savedMaximized(QPointF(0, 0))
        , savedFullScreen(QPointF(0, 0))
        , taskIconGeometry(QRect(0, 0, 32, 32))
        , active(false)
        , minimized(false)
        , maximized(false)
        , fullScreen(false)
        , moveItem(new QQuickItem())
        , designedOutput(Q_NULLPTR)
    {}

    WindowManager *wm;
    QByteArray interfaceName;
    QWaylandSurface *surface;
    ClientWindow::Type type;
    ClientWindow *parentWindow;
    QString title;
    QString appId;
    qint64 pid;
    QString iconName;
    qreal x, y;
    QPointF savedMaximized;
    QPointF savedFullScreen;
    QRect windowGeometry;
    QRect taskIconGeometry;
    bool active;
    bool minimized;
    bool maximized;
    bool fullScreen;

    QQuickItem *moveItem;
    QVector<ClientWindowView *> views;

    QWaylandOutput *designedOutput;
    QList<QWaylandOutput *> outputsList;

    QPointF randomPosition() const;

    void findOutputs();

    void setSurface(QWaylandSurface *surface);
    void setType(ClientWindow::Type type);
    void setParentWindow(ClientWindow *window);
    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setActive(bool active);
    void setWindowGeometry(const QRect &geometry);
    void setMaximized(QWaylandOutput *output);
    void unsetMaximized();
    void setFullScreen(QWaylandOutput *output);
    void unsetFullScreen();

    void setTopLevel();
    void setTransient(ClientWindow *parentWindow,
                      const QPoint &relativeToParent,
                      bool keyboardFocus);
    void setPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parentSurface, const QPoint &relativeToParent);

    QString findDesktopFile(const QString &appId);

    QQmlListProperty<ClientWindowView> windowViews();

    static int windowViewsCount(QQmlListProperty<ClientWindowView> *prop);
    static ClientWindowView *windowViewsAt(QQmlListProperty<ClientWindowView> *prop, int index);

    QQmlListProperty<QWaylandOutput> outputs();

    static int outputsCount(QQmlListProperty<QWaylandOutput> *prop);
    static QWaylandOutput *outputsAt(QQmlListProperty<QWaylandOutput> *prop, int index);

    static ClientWindowPrivate *get(ClientWindow *clientWindow) { return clientWindow->d_func(); }
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_CLIENTWINDOW_P_H
