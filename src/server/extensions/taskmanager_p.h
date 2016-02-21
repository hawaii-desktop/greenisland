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

#ifndef GREENISLAND_TASKMANAGER_P_H
#define GREENISLAND_TASKMANAGER_P_H

#include <GreenIsland/QtWaylandCompositor/private/qwaylandextension_p.h>

#include <GreenIsland/Server/ClientWindow>
#include <GreenIsland/Server/TaskManager>
#include <GreenIsland/server/private/qwayland-server-greenisland.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace GreenIsland {

namespace Server {

class GREENISLANDSERVER_EXPORT TaskManagerPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_windows
{
    Q_DECLARE_PUBLIC(TaskManager)
public:
    TaskManagerPrivate();
    ~TaskManagerPrivate();

    bool initialized;
    Resource *boundResource;
    QMap<ClientWindow *, TaskItem *> map;

    void windowMapped(ClientWindow *window);
    void windowUnmapped(ClientWindow *window);

    static TaskManagerPrivate *get(TaskManager *tm) { return tm->d_func(); }

protected:
    void windows_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
};

class GREENISLANDSERVER_EXPORT TaskItemPrivate
        : public QWaylandExtensionTemplatePrivate
        , public QtWaylandServer::greenisland_window
{
    Q_DECLARE_PUBLIC(TaskItem)
public:
    TaskItemPrivate(ClientWindow *w);

    ClientWindow *window;
    uint32_t windowType;
    uint32_t windowState;

    void determineType();
    void determineState();

    static TaskItemPrivate *get(TaskItem *t) { return t->d_func(); }
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_TASKMANAGER_P_H
