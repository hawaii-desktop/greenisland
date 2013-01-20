/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStringList>

#include <VCompositorPlugin>

#include "greenisland.h"
#include "cmakedirs.h"

GreenIsland::GreenIsland(int &argc, char **argv)
    : QGuiApplication(argc, argv)
{
    // Set application information
    setApplicationName("Green Island");
    setApplicationVersion("0.1.0");
    setOrganizationName("Maui Project");
    setOrganizationDomain("maui-project.org");
}

VCompositor *GreenIsland::loadCompositor(const QString &name)
{
    // Load plugins
    QDir pluginsDir(QStringLiteral("%1/greenisland/compositors").arg(INSTALL_PLUGINSDIR));
    foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
        qDebug() << "Trying" << fileName << "plugin";

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QJsonValue keys = loader.metaData()["MetaData"];

        if (keys.toObject()["Keys"].toArray().contains(QJsonValue(name))) {
            VCompositorPlugin *plugin = qobject_cast<VCompositorPlugin *>(
                        loader.instance());
            if (!plugin)
                continue;

            return plugin->create(name);
        }
    }

    return 0;
}

#include "moc_greenisland.cpp"
