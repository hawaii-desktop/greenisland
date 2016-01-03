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

#include <GreenIsland/Server/AbstractPlugin>
#include <GreenIsland/Server/Compositor>

#include "plasmaeffects.h"
#include "plasmashell.h"

namespace GreenIsland {

class GREENISLANDSERVER_EXPORT PlasmaPlugin : public AbstractPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.hawaiios.greenisland.Plugin")
    Q_INTERFACES(GreenIsland::AbstractPlugin)
public:
    PlasmaPlugin()
        : AbstractPlugin()
        , m_compositor(Q_NULLPTR)
    {
    }

    QString name() const
    {
        return QStringLiteral("plasma");
    }

    void registerPlugin(Compositor *compositor)
    {
        m_compositor = compositor;
    }

    void addGlobalInterfaces()
    {
        PlasmaShell *plasmaShell = new PlasmaShell(m_compositor);
        m_compositor->addGlobalInterface(plasmaShell);
        m_compositor->addGlobalInterface(new PlasmaEffects(plasmaShell));
    }

private:
    Compositor *m_compositor;
};

}

#include "plugin.moc"
