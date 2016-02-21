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
