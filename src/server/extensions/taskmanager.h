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
