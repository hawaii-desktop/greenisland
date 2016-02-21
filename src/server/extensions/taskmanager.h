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

#ifndef GREENISLAND_TASKMANAGER_H
#define GREENISLAND_TASKMANAGER_H

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class TaskItem;
class TaskItemPrivate;
class TaskManagerPrivate;

class GREENISLANDSERVER_EXPORT TaskManager : public QWaylandExtensionTemplate<TaskManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TaskManager)
public:
    TaskManager();
    TaskManager(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

class GREENISLANDSERVER_EXPORT TaskItem : public QWaylandExtensionTemplate<TaskItem>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TaskItem)
public:
    static const struct wl_interface *interface();
    static QByteArray interfaceName();

private:
    TaskItem();

    friend class TaskManagerPrivate;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_TASKMANAGER_H
