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
#include <GreenIsland/Output>
#include <GreenIsland/QuickSurface>
#include <GreenIsland/WindowView>
#include <GreenIsland/ShellWindowView>

#include "fpscounter.h"

using namespace GreenIsland;

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
    qmlRegisterType<Compositor>(uri, 1, 0, "Compositor");
    qmlRegisterUncreatableType<ApplicationManager>(uri, 1, 0, "ApplicationManager",
                                                   QStringLiteral("You can't create ApplicationManager objects"));
    qmlRegisterUncreatableType<ClientWindow>(uri, 1, 0, "ClientWindow",
                                             QStringLiteral("You can't create ClientWindow objects"));
    qmlRegisterUncreatableType<Output>(uri, 1, 0, "Output",
                                       QStringLiteral("You can't create Output objects"));
    qmlRegisterUncreatableType<QuickSurface>(uri, 1, 0, "QuickSurface",
                                             QStringLiteral("You can't create QuickSurface objects"));
    qmlRegisterUncreatableType<WindowView>(uri, 1, 0, "WindowView",
                                           QStringLiteral("You can't create WindowView objects"));
    qmlRegisterUncreatableType<ShellWindowView>(uri, 1, 0, "ShellWindowView",
                                                QStringLiteral("You can't create ShellWindowView objects"));
    qmlRegisterType<FpsCounter>(uri, 1, 0, "FpsCounter");
}

#include "plugin.moc"
