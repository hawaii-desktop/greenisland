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

#ifndef GREENISLAND_APPLICATIONMANAGER_P_H
#define GREENISLAND_APPLICATIONMANAGER_P_H

#include <QtCore/QMultiMap>

#include <GreenIsland/QtWaylandCompositor/private/qwaylandextension_p.h>

#include <GreenIsland/Server/ApplicationManager>
#include <GreenIsland/server/private/qwayland-server-greenisland.h>

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

namespace GreenIsland {

namespace Server {

class ClientWindow;

class GREENISLANDSERVER_EXPORT ApplicationManagerPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_applications
{
    Q_DECLARE_PUBLIC(ApplicationManager)
public:
    ApplicationManagerPrivate();

    void registerWindow(ClientWindow *window);
    void unregisterWindow(ClientWindow *window);

    void _q_appIdChanged();
    void _q_activeChanged();

    static ApplicationManagerPrivate *get(ApplicationManager *appMan) { return appMan->d_func(); }

    QMultiMap<QString, ClientWindow *> appIdMap;

protected:
    void applications_quit(Resource *resource, const QString &app_id) Q_DECL_OVERRIDE;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_APPLICATIONMANAGER_P_H

