/****************************************************************************
 * This file is part of Hawaii.
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

#include <GreenIsland/Platform/EGLDeviceIntegration>

#include "eglfsmaliintegration.h"

using namespace GreenIsland::Platform;

class EglFSMaliIntegrationPlugin : public EGLDeviceIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GreenIslandDeviceIntegrationFactoryInterface_iid FILE "mali.json")
public:
    EGLDeviceIntegration *create() Q_DECL_OVERRIDE;
};

EGLDeviceIntegration *EglFSMaliIntegrationPlugin::create()
{
    return new EglFSMaliIntegration;
}

#include "main.moc"
