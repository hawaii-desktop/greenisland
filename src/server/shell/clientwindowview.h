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

#ifndef GREENISLAND_CLIENTWINDOWVIEW_H
#define GREENISLAND_CLIENTWINDOWVIEW_H

#include <QtQml/QQmlPropertyMap>

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickOutput>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class ClientWindow;
class ClientWindowViewPrivate;

class GREENISLANDSERVER_EXPORT ClientWindowView : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClientWindowView)
    Q_PROPERTY(QWaylandQuickOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(QWaylandQuickItem *shellSurfaceItem READ shellSurfaceItem NOTIFY shellSurfaceItemChanged)
    Q_PROPERTY(QQmlPropertyMap *savedProperties READ savedProperties NOTIFY savedPropertiesChanged)
public:
    ClientWindowView(QQuickItem *parent = Q_NULLPTR);

    QWaylandQuickOutput *output() const;
    QWaylandQuickItem *shellSurfaceItem() const;
    QQmlPropertyMap *savedProperties() const;

    Q_INVOKABLE void initialize(GreenIsland::Server::ClientWindow *window, QWaylandQuickOutput *output);

Q_SIGNALS:
    void outputChanged();
    void shellSurfaceItemChanged();
    void savedPropertiesChanged();

protected:
    bool childMouseEventFilter(QQuickItem *item, QEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void shellSurfaceItemWidthChanged();
    void shellSurfaceItemHeightChanged();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_CLIENTWINDOWVIEW_H
