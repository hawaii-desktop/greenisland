/****************************************************************************
 * This file is part of Green Island.
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

#ifndef GREENISLAND_CLIENTWINDOWVIEW_H
#define GREENISLAND_CLIENTWINDOWVIEW_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class ClientWindow;
class ClientWindowViewPrivate;

class GREENISLANDSERVER_EXPORT ClientWindowView : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClientWindowView)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(QWaylandQuickItem *shellSurfaceItem READ shellSurfaceItem NOTIFY shellSurfaceItemChanged)
public:
    ClientWindowView(QQuickItem *parent = Q_NULLPTR);

    QWaylandOutput *output() const;
    QWaylandQuickItem *shellSurfaceItem() const;

    Q_INVOKABLE void initialize(GreenIsland::Server::ClientWindow *window, QWaylandOutput *output);

Q_SIGNALS:
    void outputChanged();
    void shellSurfaceItemChanged();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void shellSurfaceItemWidthChanged();
    void shellSurfaceItemHeightChanged();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_CLIENTWINDOWVIEW_H