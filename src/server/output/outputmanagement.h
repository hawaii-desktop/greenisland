/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#ifndef GREENISLAND_OUTPUTMANAGEMENT_H
#define GREENISLAND_OUTPUTMANAGEMENT_H

#include <QtCore/QObject>

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>
#include <GreenIsland/QtWaylandCompositor/QWaylandResource>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class OutputConfiguration;
class OutputManagementPrivate;

class GREENISLANDSERVER_EXPORT OutputManagement : public QWaylandExtensionTemplate<OutputManagement>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OutputManagement)
public:
    OutputManagement();
    explicit OutputManagement(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void createOutputConfiguration(const QWaylandResource &resource);
    void outputConfigurationCreated(GreenIsland::Server::OutputConfiguration *configuration);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_OUTPUTMANAGEMENT_H
