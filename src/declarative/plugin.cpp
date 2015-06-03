/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtQml/QQmlExtensionPlugin>
#include <QtQml/QQmlComponent>

#include <GreenIsland/ApplicationManager>
#include <GreenIsland/ClientWindow>
#include <GreenIsland/Compositor>
#include <GreenIsland/CompositorSettings>
#include <GreenIsland/Output>
#include <GreenIsland/KeyBindings>
#include <GreenIsland/ShellWindow>

#include "fpscounter.h"

using namespace GreenIsland;

static QObject *compositorProvider(QQmlEngine *, QJSEngine *)
{
    return Compositor::instance();
}

static QObject *keyBindingsProvider(QQmlEngine *, QJSEngine *)
{
    return KeyBindings::instance();
}

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
    qmlRegisterSingletonType<Compositor>(uri, 1, 0, "Compositor", compositorProvider);
    qmlRegisterUncreatableType<ApplicationManager>(uri, 1, 0, "ApplicationManager",
                                                   QStringLiteral("You can't create ApplicationManager objects"));
    qmlRegisterUncreatableType<ClientWindow>(uri, 1, 0, "ClientWindow",
                                             QStringLiteral("You can't create ClientWindow objects"));
    qmlRegisterUncreatableType<CompositorSettings>(uri, 1, 0, "CompositorSettings",
                                                   QStringLiteral("You can't create CompositorSettings objects"));
    qmlRegisterUncreatableType<Output>(uri, 1, 0, "Output",
                                       QStringLiteral("You can't create Output objects"));
    qmlRegisterUncreatableType<ShellWindow>(uri, 1, 0, "ShellWindow",
                                            QStringLiteral("You can't create ShellWindow objects"));
    qmlRegisterType<FpsCounter>(uri, 1, 0, "FpsCounter");
    qmlRegisterSingletonType<KeyBindings>(uri, 1, 0, "KeyBindings", keyBindingsProvider);
}

#include "plugin.moc"
