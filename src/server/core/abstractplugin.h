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

#ifndef GREENISLAND_ABSTRACTPLUGIN_H
#define GREENISLAND_ABSTRACTPLUGIN_H

#include <QtCore/QObject>

#include <GreenIsland/server/greenislandserver_export.h>

class QWaylandCompositor;

namespace GreenIsland {

namespace Server {

/*!
 * @brief Abstract plugin class.
 */
class GREENISLANDSERVER_EXPORT AbstractPlugin : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractPlugin();

    /*!
     * \return Returns a user-friendly name for the plugin.
     */
    virtual QString name() const = 0;

    /*!
     * \brief  Registers plugin.
     * \param compositor The compositor instance.
     *
     * Register the plugin within the compositor.
     * At this point the compositor still has to perform some actions,
     * such as acquiring the screen configuration and creating outputs.
     *
     * When implementing this method, the plugin can connect to the
     * Compositor signals or save the pointer to do something later.
     */
    virtual void registerPlugin(QWaylandCompositor *compositor) = 0;
};

} // namespace Server

} // namespace GreenIsland

Q_DECLARE_INTERFACE(GreenIsland::Server::AbstractPlugin, "org.hawaiios.greenisland.Plugin")

#endif // GREENISLAND_ABSTRACTPLUGIN_H
