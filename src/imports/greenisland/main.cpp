/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>

#include <QtAccountsService/AccountsManager>

#include <VIndicator>

#include "enums.h"
#include "indicatorsmodel.h"
#include "launcherdropitem.h"
#include "listaggregatormodel.h"
#include "appsmodel.h"
#include "appchoosercategories.h"
#include "launchermodel.h"
#include "launcherapplication.h"
#include "launcherurl.h"

class GreenIslandQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) {
        Q_ASSERT(uri == QLatin1String("GreenIsland"));

        // Indicators
        qmlRegisterType<IndicatorsModel>(uri, 1, 0, "IndicatorsModel");
        qmlRegisterType<VIndicator>(uri, 1, 0, "Indicator");

        // Launcher with drag & drop support
        qmlRegisterType<LauncherDropItem>(uri, 1, 0, "LauncherDropItem");

        // Models
        qmlRegisterType<ListAggregatorModel>(uri, 1, 0, "ListAggregatorModel");
        qmlRegisterType<AppChooserCategories>(uri, 1, 0, "AppChooserCategoriesModel");
        qmlRegisterType<AppsModel>(uri, 1, 0, "AvailableApplicationsModel");
        qmlRegisterType<AppsModelSorted>(uri, 1, 0, "AvailableApplicationsModelSorted");

        // Launcher model
        qmlRegisterType<LauncherModel>(uri, 1, 0, "LauncherModel");

        // Launcher item types
        qmlRegisterType<LauncherApplication>(uri, 1, 0, "LauncherApplication");
        qmlRegisterType<LauncherUrl>(uri, 1, 0, "LauncherUrl");

        // Enums
        qmlRegisterUncreatableType<LauncherAlignment>(uri, 1, 0, "LauncherAlignment", "");
        qmlRegisterUncreatableType<UserStatus>(uri, 1, 0, "UserStatus", "");
    }

    void initializeEngine(QQmlEngine *engine, const char *uri) {
        Q_ASSERT(uri == QLatin1String("GreenIsland"));

        QQmlContext *context = engine->rootContext();

        // Register a global accounts manager object
        QtAddOn::AccountsService::AccountsManager *accountsManager =
            new QtAddOn::AccountsService::AccountsManager();
        context->setContextProperty("accountsManager", accountsManager);
    }
};

#include "main.moc"
