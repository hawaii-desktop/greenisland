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

#ifndef GREENISLAND_OUTPUTCONFIGURATION_H
#define GREENISLAND_OUTPUTCONFIGURATION_H

#include <QtCore/QObject>

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>
#include <GreenIsland/QtWaylandCompositor/QWaylandResource>

#include <GreenIsland/server/greenislandserver_export.h>

QT_BEGIN_NAMESPACE

class QWaylandOutput;

QT_END_NAMESPACE

namespace GreenIsland {

namespace Server {

class OutputChangeset;
class OutputConfigurationPrivate;
class OutputManagement;
class OutputManagementPrivate;

class GREENISLANDSERVER_EXPORT OutputConfiguration
        : public QWaylandExtensionTemplate<OutputConfiguration>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OutputConfiguration)
public:
    OutputConfiguration();
    explicit OutputConfiguration(OutputManagement *parent, const QWaylandResource &resource);
    virtual ~OutputConfiguration();

    Q_INVOKABLE void initialize(GreenIsland::Server::OutputManagement *parent, const QWaylandResource &resource);
    
    QHash<QWaylandOutput *, OutputChangeset *> changes() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

    static OutputConfiguration *fromResource(wl_resource *resource);

Q_SIGNALS:
    void changeRequested();

public Q_SLOTS:
    void setApplied();
    void setFailed();

private:
    void initialize();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_OUTPUTCONFIGURATION_H
