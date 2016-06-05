/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#include <QtQml/QQmlExtensionPlugin>
#include <QtQml/QQmlComponent>

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickSurface>
#include <GreenIsland/QtWaylandCompositor/QWaylandClient>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickOutput>
#include <GreenIsland/QtWaylandCompositor/QWaylandCompositorExtension>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickExtension>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <GreenIsland/QtWaylandCompositor/QWaylandResource>
#include <GreenIsland/QtWaylandCompositor/QWaylandQtWindowManager>
#include <GreenIsland/QtWaylandCompositor/QWaylandWlShell>
#include <GreenIsland/QtWaylandCompositor/QWaylandTextInputManager>
#include <GreenIsland/QtWaylandCompositor/QWaylandXdgShell>

#include <GreenIsland/Server/ApplicationManager>
#include <GreenIsland/Server/CompositorSettings>
#include <GreenIsland/Server/OutputChangeset>
#include <GreenIsland/Server/OutputManagement>
#include <GreenIsland/Server/QuickOutput>
#include <GreenIsland/Server/QuickOutputConfiguration>
#include <GreenIsland/Server/GtkShell>
#include <GreenIsland/Server/Keymap>
#include <GreenIsland/Server/Screen>
#include <GreenIsland/Server/Screencaster>
#include <GreenIsland/Server/Screenshooter>
//#include <GreenIsland/Server/TaskManager>
#include <GreenIsland/Server/QuickScreenManager>

#include "fpscounter.h"
#include "keyeventfilter.h"
#include "qwaylandmousetracker_p.h"

using namespace GreenIsland::Server;

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CONTAINER_CLASS(QWaylandQuickCompositor)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandQtWindowManager)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandWlShell)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandXdgShell)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandTextInputManager)

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(GtkShell)

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(OutputManagement)

class GreenIslandPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
    void registerTypes(const char *uri);
};

void GreenIslandPlugin::registerTypes(const char *uri)
{
    // @uri GreenIsland
    Q_ASSERT(QLatin1String(uri) == QLatin1String("GreenIsland"));

    // Base types
    qmlRegisterType<QWaylandQuickCompositorQuickExtensionContainer>(uri, 1, 0, "WaylandCompositor");
    qmlRegisterType<QWaylandQuickItem>(uri, 1, 0, "WaylandQuickItem");
    qmlRegisterType<QWaylandMouseTracker>(uri, 1, 0, "WaylandMouseTracker");
    qmlRegisterType<QWaylandQuickOutput>(uri, 1, 0, "WaylandOutput");
    qmlRegisterType<QWaylandQuickSurface>(uri, 1, 0, "WaylandSurface");

    // Uncreatable types
    qmlRegisterUncreatableType<QWaylandCompositorExtension>(uri, 1, 0, "WaylandExtension",
                                                            QObject::tr("Cannot create instance of WaylandExtension"));
    qmlRegisterUncreatableType<QWaylandClient>(uri, 1, 0, "WaylandClient",
                                               QObject::tr("Cannot create instance of WaylandClient"));
    qmlRegisterUncreatableType<QWaylandOutput>(uri, 1, 0, "WaylandOutputBase",
                                               QObject::tr("Cannot create instance of WaylandOutputBase, use WaylandOutput instead"));
    qmlRegisterUncreatableType<QWaylandView>(uri, 1, 0, "WaylandView",
                                             QObject::tr("Cannot create instance of WaylandView, it can be retrieved by accessor on WaylandQuickItem"));
    qmlRegisterUncreatableType<QWaylandInputDevice>(uri, 1, 0, "WaylandInputDevice",
                                                    QObject::tr("Cannot create instance of WaylandInputDevice"));
    qmlRegisterUncreatableType<QWaylandCompositor>(uri, 1, 0, "WaylandCompositorBase",
                                                   QObject::tr("Cannot create instance of WaylandCompositorBase, use WaylandCompositor instead"));
    qmlRegisterUncreatableType<QWaylandSurface>(uri, 1, 0, "WaylandSurfaceBase",
                                                QObject::tr("Cannot create instance of WaylandSurfaceBase, use WaylandSurface instead"));
    qmlRegisterUncreatableType<QWaylandShellSurface>(uri, 1, 0, "ShellSurface",
                                                     QObject::tr("Cannot create instance of ShellSurface"));
    qmlRegisterUncreatableType<QWaylandWlShellSurface>(uri, 1, 0, "WlShellSurfaceBase",
                                                       QObject::tr("Cannot create instance of WlShellSurfaceBase, use WlShellSurface instead"));
    qmlRegisterUncreatableType<QWaylandXdgSurface>(uri, 1, 0, "XdgSurfaceBase",
                                                   QObject::tr("Cannot create instance of XdgSurfaceBase, use XdgSurface instead"));
    qmlRegisterUncreatableType<QWaylandResource>(uri, 1, 0, "WaylandResource",
                                                 QObject::tr("Cannot create instance of WaylandResource"));

    // Extensions
    qmlRegisterType<QWaylandQtWindowManagerQuickExtension>(uri, 1, 0, "QtWindowManager");
    qmlRegisterType<QWaylandWlShellQuickExtension>(uri, 1, 0, "WlShell");
    qmlRegisterType<QWaylandWlShellSurface>(uri, 1, 0, "WlShellSurface");
    qmlRegisterType<QWaylandQuickShellSurfaceItem>(uri, 1, 0, "ShellSurfaceItem");
    qmlRegisterType<QWaylandXdgShellQuickExtension>(uri, 1, 0, "XdgShell");
    qmlRegisterType<QWaylandXdgSurface>(uri, 1, 0, "XdgSurface");
    qmlRegisterType<QWaylandTextInputManagerQuickExtension>(uri, 1, 0, "TextInputManager");

    // More specialized output
    qmlRegisterType<QuickOutput>(uri, 1, 0, "ExtendedOutput");

    // gtk-shell
    qmlRegisterType<GtkShellQuickExtension>(uri, 1, 0, "GtkShell");
    qmlRegisterType<GtkSurface>(uri, 1, 0, "GtkSurface");

    // Screen
    qmlRegisterType<QuickScreenManager>(uri, 1, 0, "ScreenManager");
    qmlRegisterUncreatableType<Screen>(uri, 1, 0, "Screen",
                                       QObject::tr("Cannot create instance of Screen"));

    // Output management
    qmlRegisterType<QuickOutputConfiguration>(uri, 1, 0, "OutputConfiguration");
    qmlRegisterUncreatableType<OutputManagement>(uri, 1, 0, "OutputManagementBase",
                                                 QObject::tr("Cannot create instance of OutputManagementBase, use OutputManagement instead"));
    qmlRegisterType<OutputManagementQuickExtension>(uri, 1, 0, "OutputManagement");
    qmlRegisterUncreatableType<OutputChangeset>(uri, 1, 0, "OutputChangeset",
                                                QObject::tr("Cannot create instance of OutputChangeset"));

    // Application manager
    qmlRegisterType<ApplicationManager>(uri, 1, 0, "ApplicationManager");

    // Task manager
    //qmlRegisterType<TaskManager>(uri, 1, 0, "TaskManager");
    //qmlRegisterUncreatableType<TaskItem>(uri, 1, 0, "TaskItem",
                                         //QObject::tr("Cannot create instance of TaskItem"));

    // Screencaster
    qmlRegisterType<Screencaster>(uri, 1, 0, "Screencaster");
    qmlRegisterUncreatableType<Screencast>(uri, 1, 0, "Screencast",
                                           QObject::tr("Cannot create instance of Screencast"));

    // Screen shooter
    qmlRegisterType<Screenshooter>(uri, 1, 0, "Screenshooter");
    qmlRegisterUncreatableType<Screenshot>(uri, 1, 0, "Screenshot",
                                           QObject::tr("Cannot create instance of Screenshot"));

    // Key event filter
    qmlRegisterType<KeyEventFilter>(uri, 1, 0, "KeyEventFilter");

    // Keymap
    qmlRegisterType<Keymap>(uri, 1, 0, "Keymap");

    // Settings
    qmlRegisterType<CompositorSettings>(uri, 1, 0, "CompositorSettings");

    // Misc
    qmlRegisterType<FpsCounter>(uri, 1, 0, "FpsCounter");
}

#include "plugin.moc"
