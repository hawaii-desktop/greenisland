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
