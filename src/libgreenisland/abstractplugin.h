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
     * \brief Starts plugin's business.
     * \param compositor The compositor instance.
     */
    virtual void start(Compositor *compositor) = 0;
};

}

Q_DECLARE_INTERFACE(GreenIsland::AbstractPlugin, "org.greenisland.Plugin")

#endif // ABSTRACTPLUGIN_H
