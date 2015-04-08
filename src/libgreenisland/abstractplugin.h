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

#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include <GreenIsland/Compositor>

namespace GreenIsland {

/*!
 * @brief Abstract plugin class.
 */
class GREENISLAND_EXPORT AbstractPlugin : public QObject
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
    virtual void registerPlugin(Compositor *compositor) = 0;

    /*!
     * \brief Add global interfaces.
     *
     * Plugins can override this method to add their global
     * interfaces in order to implement custom Wayland protocols.
     */
    virtual void addGlobalInterfaces();
};

}

Q_DECLARE_INTERFACE(GreenIsland::AbstractPlugin, "org.greenisland.Plugin")

#endif // ABSTRACTPLUGIN_H
