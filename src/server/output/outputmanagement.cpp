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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandResource>

#include "outputconfiguration.h"
#include "outputconfiguration_p.h"
#include "outputmanagement.h"
#include "outputmanagement_p.h"
#include "serverlogging_p.h"

namespace GreenIsland {

namespace Server {

/*
 * OutputManagementPrivate
 */

OutputManagementPrivate::OutputManagementPrivate()
    : QWaylandCompositorExtensionPrivate()
    , QtWaylandServer::greenisland_outputmanagement()
{
}

void OutputManagementPrivate::removeConfiguration(OutputConfiguration *configuration)
{
    configurations.removeOne(configuration);
}

void OutputManagementPrivate::outputmanagement_create_configuration(Resource *resource, uint32_t id)
{
    Q_Q(OutputManagement);

    QWaylandResource configurationResource(
                wl_resource_create(resource->client(),
                                   &greenisland_outputconfiguration_interface,
                                   wl_resource_get_version(resource->handle),
                                   id));

    Q_EMIT q->createOutputConfiguration(configurationResource);

    OutputConfiguration *configuration = OutputConfiguration::fromResource(configurationResource.resource());
    if (!configuration) {
        // An OutputConfiguration was not created in response to the createOutputConfiguration
        // signal so we create one as a fallback here instead
        configuration = new OutputConfiguration(q, configurationResource);
    }

    Q_EMIT q->outputConfigurationCreated(configuration);

    configurations.append(configuration);
}

/*
 * OutputManagement
 */

OutputManagement::OutputManagement()
    : QWaylandCompositorExtensionTemplate<OutputManagement>(*new OutputManagementPrivate())
{
}

OutputManagement::OutputManagement(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<OutputManagement>(compositor, *new OutputManagementPrivate())
{
}

void OutputManagement::initialize()
{
    Q_D(OutputManagement);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcOutputManagement) << "Failed to find QWaylandCompositor when initializing OutputManagement";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *OutputManagement::interface()
{
    return OutputManagementPrivate::interface();
}

QByteArray OutputManagement::interfaceName()
{
    return OutputManagementPrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_outputmanagement.cpp"
