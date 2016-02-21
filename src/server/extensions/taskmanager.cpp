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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>

#include "serverlogging_p.h"
#include "taskmanager.h"
#include "taskmanager_p.h"

namespace GreenIsland {

namespace Server {

/*
 * TaskManagerPrivate
 */

TaskManagerPrivate::TaskManagerPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_windows()
    , initialized(false)
    , boundResource(Q_NULLPTR)
{
}

TaskManagerPrivate::~TaskManagerPrivate()
{
    while (!map.isEmpty()) {
        TaskItem *task = map.take(map.firstKey());
        TaskItemPrivate::get(task)->send_unmapped();
        delete task;
    }
}

void TaskManagerPrivate::windowMapped(ClientWindow *window)
{
    Q_Q(TaskManager);

    // Can't continue if it's not initialized
    if (!initialized)
        return;

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());

    TaskItem *task = new TaskItem();
    TaskItemPrivate *dTask = TaskItemPrivate::get(task);
    task->initialize();
    dTask->init(compositor->display(), 1);

    map.insert(window, task);

    send_window_mapped(boundResource->handle, dTask->resource()->handle,
                       dTask->windowType, dTask->windowState,
                       window->title(), window->appId());
}

void TaskManagerPrivate::windowUnmapped(ClientWindow *window)
{
    TaskItem *task = map.take(window);
    TaskItemPrivate::get(task)->send_unmapped();
    delete task;
}

void TaskManagerPrivate::windows_bind_resource(Resource *resource)
{
    if (boundResource) {
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "greenisland_windows can only be bound once");
        return;
    }

    boundResource = resource;
}

/*
 * TaskItemPrivate
 */

TaskItemPrivate::TaskItemPrivate(ClientWindow *w)
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_window()
    , window(w)
{
    determineType();
    determineState();

    QObject::connect(w, &ClientWindow::typeChanged, [this] {
        determineType();
    });
    QObject::connect(w, &ClientWindow::titleChanged, [this] {
        send_title_changed(window->title());
    });
    QObject::connect(w, &ClientWindow::appIdChanged, [this] {
        send_title_changed(window->appId());
    });
    QObject::connect(w, &ClientWindow::activeChanged, [this] {
        determineState();
        send_state_changed(windowState);
    });
    QObject::connect(w, &ClientWindow::minimizedChanged, [this] {
        determineState();
        send_state_changed(windowState);
    });
    QObject::connect(w, &ClientWindow::maximizedChanged, [this] {
        determineState();
        send_state_changed(windowState);
    });
    QObject::connect(w, &ClientWindow::fullScreenChanged, [this] {
        determineState();
        send_state_changed(windowState);
    });
}

void TaskItemPrivate::determineType()
{
    switch (window->type()) {
    case ClientWindow::Popup:
        windowType = QtWaylandServer::greenisland_windows::type_popup;
        break;
    case ClientWindow::Transient:
        windowType = QtWaylandServer::greenisland_windows::type_transient;
        break;
    default:
        windowType = QtWaylandServer::greenisland_windows::type_toplevel;
        break;
    }
}

void TaskItemPrivate::determineState()
{
    uint32_t s = QtWaylandServer::greenisland_windows::state_inactive;

    if (window->isActive())
        s |= QtWaylandServer::greenisland_windows::state_active;
    if (window->isMinimized())
        s |= QtWaylandServer::greenisland_windows::state_minimized;
    if (window->isMaximized())
        s |= QtWaylandServer::greenisland_windows::state_maximized;
    if (window->isFullScreen())
        s |= QtWaylandServer::greenisland_windows::state_fullscreen;

    windowState = s;
}

/*
 * TaskManager
 */

TaskManager::TaskManager()
    : QWaylandExtensionTemplate<TaskManager>(*new TaskManagerPrivate())
{
}

TaskManager::TaskManager(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<TaskManager>(compositor, *new TaskManagerPrivate())
{
}

void TaskManager::initialize()
{
    Q_D(TaskManager);

    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcTaskManager) << "Failed to find QWaylandCompositor when initializing TaskManager";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *TaskManager::interface()
{
    return TaskManagerPrivate::interface();
}

QByteArray TaskManager::interfaceName()
{
    return TaskManagerPrivate::interfaceName();
}

/*
 * TaskItem
 */

TaskItem::TaskItem()
{
}

const struct wl_interface *TaskItem::interface()
{
    return TaskItemPrivate::interface();
}

QByteArray TaskItem::interfaceName()
{
    return TaskItemPrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_taskmanager.cpp"
