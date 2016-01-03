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

#ifndef XWAYLANDPLUGIN_H
#define XWAYLANDPLUGIN_H

#include <GreenIsland/Server/AbstractPlugin>

namespace GreenIsland {

class GREENISLANDSERVER_EXPORT XWaylandPlugin : public AbstractPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.greenisland.Plugin")
    Q_INTERFACES(GreenIsland::AbstractPlugin)
public:
    XWaylandPlugin();

    QString name() const;

    void registerPlugin(Compositor *compositor);
};

}

#endif // XWAYLANDPLUGIN_H
