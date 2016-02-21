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

#include <GreenIsland/QtWaylandCompositor/QWaylandClient>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickExtension>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickItem>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <GreenIsland/QtWaylandCompositor/QWaylandQuickSurface>
#include <GreenIsland/QtWaylandCompositor/QWaylandShell>

#include <GreenIsland/Server/ApplicationManager>
#include <GreenIsland/Server/CompositorSettings>
#include <GreenIsland/Server/ClientWindow>
#include <GreenIsland/Server/ClientWindowView>
#include <GreenIsland/Server/QuickOutput>
#include <GreenIsland/Server/GtkShell>
#include <GreenIsland/Server/Keymap>
#include <GreenIsland/Server/QuickXdgPopupItem>
#include <GreenIsland/Server/QuickXdgSurfaceItem>
#include <GreenIsland/Server/XdgShell>
#include <GreenIsland/Server/Screen>
#include <GreenIsland/Server/Screencaster>
#include <GreenIsland/Server/Screenshooter>
#include <GreenIsland/Server/TaskManager>
#include <GreenIsland/Server/QuickScreenManager>
#include <GreenIsland/Server/WindowManager>

#include "quickkeybindings.h"
#include "fpscounter.h"
#include "globalpointertracker.h"
#include "keybindingsfilter.h"
#include "keyeventfilter.h"
#include "localpointertracker.h"
#include "pointeritem.h"
#include "keybindingsfilter.h"
#include "keyeventfilter.h"

using namespace GreenIsland::Server;

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandQuickCompositor)
Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(QWaylandShell)
Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(QWaylandShellSurface)

Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(GtkShell)
Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(GtkSurface)

Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(XdgPopup)
Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(XdgShell)
Q_COMPOSITOR_DECLARE_QUICK_DATA_CLASS(XdgSurface)

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
    qmlRegisterType<QWaylandQuickCompositorQuickExtension>(uri, 1, 0, "WaylandCompositor");
    qmlRegisterType<QWaylandQuickItem>(uri, 1, 0, "WaylandQuickItem");
    qmlRegisterType<QuickOutput>(uri, 1, 0, "WaylandOutput");
    qmlRegisterType<QWaylandQuickSurface>(uri, 1, 0, "WaylandSurface");

    // Pointer tracking
    qmlRegisterType<GlobalPointerTracker>(uri, 1, 0, "GlobalPointerTracker");
    qmlRegisterType<LocalPointerTracker>(uri, 1, 0, "LocalPointerTracker");
    qmlRegisterType<PointerItem>(uri, 1, 0, "PointerItem");

    // Uncreatable base types
    qmlRegisterUncreatableType<QWaylandExtension>(uri, 1, 0, "WaylandExtension",
                                                  QObject::tr("Cannot create instance of WaylandExtension"));
    qmlRegisterUncreatableType<QWaylandClient>(uri, 1, 0, "WaylandClient",
                                               QObject::tr("Cannot create instance of WaylandClient"));
    qmlRegisterUncreatableType<QWaylandView>(uri, 1, 0, "WaylandView",
                                             QObject::tr("Cannot create instance of WaylandView, it can be retrieved by accessor on WaylandQuickItem"));
    qmlRegisterUncreatableType<QWaylandInputDevice>(uri, 1, 0, "WaylandInputDevice",
                                                    QObject::tr("Cannot create instance of WaylandInputDevice"));
    qmlRegisterUncreatableType<QWaylandCompositor>(uri, 1, 0, "WaylandCompositorBase",
                                                   QObject::tr("Cannot create instance of WaylandCompositorBase, use WaylandCompositor instead"));
    qmlRegisterUncreatableType<QWaylandSurface>(uri, 1, 0, "WaylandSurfaceBase",
                                                QObject::tr("Cannot create instance of WaylandSurfaceBase, use WaylandSurface instead"));

    // wl-shell
    qmlRegisterUncreatableType<QWaylandShellSurface>(uri, 1, 0, "ShellSurfaceBase",
                                                     QObject::tr("Cannot create instance of ShellSurfaceBase, use ShellSurface instead"));
    qmlRegisterType<QWaylandShellQuickData>(uri, 1, 0, "Shell");
    qmlRegisterType<QWaylandShellSurfaceQuickData>(uri, 1, 0, "ShellSurface");
    qmlRegisterType<QWaylandQuickShellSurfaceItem>(uri, 1, 0, "ShellSurfaceItem");

    // gtk-shell
    qmlRegisterType<GtkShellQuickData>(uri, 1, 0, "GtkShell");
    qmlRegisterUncreatableType<GtkSurface>(uri, 1, 0, "GtkSurfaceBase",
                                           QObject::tr("Cannot create instance of GtkSurfaceBase, use GtkSurface instead"));
    qmlRegisterType<GtkSurfaceQuickData>(uri, 1, 0, "GtkSurface");

    // xdg-shell
    qmlRegisterUncreatableType<XdgPopup>(uri, 1, 0, "XdgPopupBase",
                                         QObject::tr("Cannot create instance of XdgPopupBase, use XdgPopup instead"));
    qmlRegisterType<XdgPopupQuickData>(uri, 1, 0, "XdgPopup");
    qmlRegisterType<QuickXdgPopupItem>(uri, 1, 0, "XdgPopupItem");
    qmlRegisterType<XdgShellQuickData>(uri, 1, 0, "XdgShell");
    qmlRegisterUncreatableType<XdgSurface>(uri, 1, 0, "XdgSurfaceBase",
                                           QObject::tr("Cannot create instance of XdgSurfaceBase, use XdgSurface instead"));
    qmlRegisterType<XdgSurfaceQuickData>(uri, 1, 0, "XdgSurface");
    qmlRegisterType<QuickXdgSurfaceItem>(uri, 1, 0, "XdgSurfaceItem");

    // Screen
    qmlRegisterType<QuickScreenManager>(uri, 1, 0, "ScreenManager");
    qmlRegisterUncreatableType<Screen>(uri, 1, 0, "Screen",
                                       QObject::tr("Cannot create instance of Screen"));

    // Window manager
    qmlRegisterType<WindowManager>(uri, 1, 0, "WindowManager");
    qmlRegisterUncreatableType<ClientWindow>(uri, 1, 0, "ClientWindow",
                                             QObject::tr("Cannot create instance of ClientWindow"));
    qmlRegisterType<ClientWindowView>(uri, 1, 0, "ClientWindowView");

    // Application manager
    qmlRegisterType<ApplicationManager>(uri, 1, 0, "ApplicationManager");

    // Task manager
    qmlRegisterType<TaskManager>(uri, 1, 0, "TaskManager");
    qmlRegisterUncreatableType<TaskItem>(uri, 1, 0, "TaskItem",
                                         QObject::tr("Cannot create instance of TaskItem"));

    // Screencaster
    qmlRegisterType<Screencaster>(uri, 1, 0, "Screencaster");
    qmlRegisterUncreatableType<Screencast>(uri, 1, 0, "Screencast",
                                           QObject::tr("Cannot create instance of Screencast"));

    // Screen shooter
    qmlRegisterType<Screenshooter>(uri, 1, 0, "Screenshooter");
    qmlRegisterUncreatableType<Screenshot>(uri, 1, 0, "Screenshot",
                                           QObject::tr("Cannot create instance of Screenshot"));

    // Key bindings
    qmlRegisterUncreatableType<KeyBinding>(uri, 1, 0, "KeyBinding",
                                           QObject::tr("Cannot create instance of KeyBinding"));
    qmlRegisterType<QuickKeyBindings>(uri, 1, 0, "KeyBindings");
    qmlRegisterType<KeyBindingsFilter>(uri, 1, 0, "KeyBindingsFilter");

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
