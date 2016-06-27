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

#ifndef GREENISLAND_CLIENTWINDOWQUICKITEM_H
#define GREENISLAND_CLIENTWINDOWQUICKITEM_H

#include <QtQml/QQmlPropertyMap>

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickShellSurfaceItem>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class ClientWindow;
class ClientWindowQuickItemPrivate;

class GREENISLANDSERVER_EXPORT ClientWindowQuickItem : public QWaylandQuickShellSurfaceItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClientWindowQuickItem)
    Q_PROPERTY(QQmlPropertyMap *savedProperties READ savedProperties NOTIFY savedPropertiesChanged)
public:
    ClientWindowQuickItem(QQuickItem *parent = Q_NULLPTR);

    QQmlPropertyMap *savedProperties() const;

Q_SIGNALS:
    void savedPropertiesChanged();

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_CLIENTWINDOWQUICKITEM_H
