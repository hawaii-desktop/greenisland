/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QRegularExpression>

#include "logging.h"
#include "serverbufferintegrationfactory_p.h"

namespace GreenIsland {

ServerBufferIntegrationInterface *ServerBufferIntegrationFactory::loadPlugin()
{
    // Detect the plugin from the environment, or fallback to the default
    QByteArray pluginToLoad = qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION");
    if (pluginToLoad.isEmpty())
        return Q_NULLPTR;

    const QStringList paths = QCoreApplication::libraryPaths();
    qCDebug(GREENISLAND_COMPOSITOR) << "Server buffer integration plugin lookup paths:" << qPrintable(paths.join(' '));

    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QStringLiteral("/greenisland/hardware-integration/server/"),
                       QStringLiteral("*.so"),
                       QDir::SortFlags(QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList infoList = dir.entryInfoList();

        Q_FOREACH (const QFileInfo &info, infoList) {
            // Load only the plugins from the environment variable
            QRegularExpression rx(QStringLiteral("(?:lib|)(.+)\\.so"));
            QRegularExpressionMatch match = rx.match(info.fileName());
            if (!match.hasMatch() || match.captured(1) != pluginToLoad) {
                qCDebug(GREENISLAND_COMPOSITOR) << "Skipping" << info.filePath();
                continue;
            }

            qCDebug(GREENISLAND_COMPOSITOR) << "Trying" << info.filePath();
            QPluginLoader loader(info.filePath());
            loader.load();

            QObject *instance = loader.instance();
            if (!instance) {
                qCWarning(GREENISLAND_COMPOSITOR, "Plugin loading failed: %s",
                          qPrintable(loader.errorString()));
                loader.unload();
                continue;
            }

            ServerBufferIntegrationInterface *plugin = qobject_cast<ServerBufferIntegrationInterface *>(instance);
            if (plugin) {
                qCDebug(GREENISLAND_COMPOSITOR) << "Loading" << plugin->name() << "plugin";
                return plugin;
            } else {
                qCWarning(GREENISLAND_COMPOSITOR,
                          "Plugin instantiation failed: \"%s\" doesn't provide a valid plugin",
                          qPrintable(info.fileName()));
            }
        }
    }

    return Q_NULLPTR;
}

} // namespace GreenIsland
