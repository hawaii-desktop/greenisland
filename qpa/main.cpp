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

#include <QtGui/qpa/qplatformintegrationplugin.h>

#include <GreenIsland/Platform/EglFSIntegration>

class GreenIslandIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "greenisland.json")
public:
    QPlatformIntegration *create(const QString &system,
                                 const QStringList &paramList) Q_DECL_OVERRIDE;
    QPlatformIntegration *create(const QString &system,
                                 const QStringList &paramList,
                                 int &argc, char **argv) Q_DECL_OVERRIDE;
};

QPlatformIntegration *GreenIslandIntegrationPlugin::create(const QString &system,
                                                           const QStringList &paramList)
{
    Q_UNUSED(paramList);

    if (system.compare(QLatin1String("greenisland"), Qt::CaseInsensitive) == 0)
        return new GreenIsland::Platform::EglFSIntegration;

    return Q_NULLPTR;
}

QPlatformIntegration *GreenIslandIntegrationPlugin::create(const QString &system,
                                                           const QStringList &paramList,
                                                           int &argc, char **argv)
{
    Q_UNUSED(paramList);
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    if (system.compare(QLatin1String("greenisland"), Qt::CaseInsensitive) == 0)
        return new GreenIsland::Platform::EglFSIntegration;

    return Q_NULLPTR;
}

#include "main.moc"
