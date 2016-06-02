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

#include <GreenIsland/QtWaylandCompositor/private/qwaylandoutput_p.h>

#include "outputchangeset.h"
#include "outputchangeset_p.h"
#include "outputconfiguration.h"
#include "outputconfiguration_p.h"
#include "outputmanagement.h"
#include "outputmanagement_p.h"

namespace GreenIsland {

namespace Server {

/*
 * OutputConfigurationPrivate
 */

OutputConfigurationPrivate::OutputConfigurationPrivate()
    : QWaylandCompositorExtensionPrivate()
    , QtWaylandServer::greenisland_outputconfiguration()
    , management(Q_NULLPTR)
{
}

OutputChangeset *OutputConfigurationPrivate::pendingChanges(QWaylandOutput *output)
{
    Q_Q(OutputConfiguration);

    if (!changes.value(output, Q_NULLPTR))
        changes[output] = new OutputChangeset(output, q);
    return changes[output];
}

bool OutputConfigurationPrivate::hasPendingChanges(QWaylandOutput *output) const
{
    if (!changes.value(output, Q_NULLPTR))
        return false;

    OutputChangeset *changeset = changes[output];
    return changeset->isEnabledChanged() ||
            changeset->isPrimaryChanged() ||
            changeset->isModeIdChanged() ||
            changeset->isTransformChanged() ||
            changeset->isPositionChanged() ||
            changeset->isScaleFactorChanged();
}

void OutputConfigurationPrivate::clearPendingChanges()
{
    qDeleteAll(changes.begin(), changes.end());
    changes.clear();
}

void OutputConfigurationPrivate::outputconfiguration_enable(Resource *resource,
                                                            struct ::wl_resource *outputResource,
                                                            int32_t enable)
{
    Q_UNUSED(resource);
    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    OutputChangesetPrivate::get(pendingChanges(output))->enabled = enable == 1;
}

void OutputConfigurationPrivate::outputconfiguration_primary(Resource *resource,
                                                            struct ::wl_resource *outputResource,
                                                            int32_t primary)
{
    Q_UNUSED(resource);
    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    OutputChangesetPrivate::get(pendingChanges(output))->primary = primary == 1;
}

void OutputConfigurationPrivate::outputconfiguration_mode(Resource *resource,
                                                          struct ::wl_resource *outputResource,
                                                          int32_t mode_id)
{
    Q_UNUSED(resource);
    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    OutputChangesetPrivate::get(pendingChanges(output))->modeId = mode_id;
}

void OutputConfigurationPrivate::outputconfiguration_transform(Resource *resource,
                                                               struct ::wl_resource *outputResource,
                                                               int32_t wlTransform)
{
    Q_UNUSED(resource);

    QWaylandOutput::Transform transform;

    switch (wlTransform) {
    case QWaylandOutputPrivate::transform_90:
        transform = QWaylandOutput::Transform90;
        break;
    case QWaylandOutputPrivate::transform_180:
        transform = QWaylandOutput::Transform180;
        break;
    case QWaylandOutputPrivate::transform_270:
        transform = QWaylandOutput::Transform270;
        break;
    case QWaylandOutputPrivate::transform_flipped:
        transform = QWaylandOutput::TransformFlipped;
        break;
    case QWaylandOutputPrivate::transform_flipped_90:
        transform = QWaylandOutput::TransformFlipped90;
        break;
    case QWaylandOutputPrivate::transform_flipped_180:
        transform = QWaylandOutput::TransformFlipped180;
        break;
    case QWaylandOutputPrivate::transform_flipped_270:
        transform = QWaylandOutput::TransformFlipped270;
        break;
    default:
        transform = QWaylandOutput::TransformNormal;
        break;
    }

    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    OutputChangesetPrivate::get(pendingChanges(output))->transform = transform;
}

void OutputConfigurationPrivate::outputconfiguration_position(Resource *resource,
                                                              struct ::wl_resource *outputResource,
                                                              int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    OutputChangesetPrivate::get(pendingChanges(output))->position = QPoint(x, y);
}

void OutputConfigurationPrivate::outputconfiguration_scale(Resource *resource,
                                                           struct ::wl_resource *outputResource,
                                                           int32_t scale)
{
    Q_UNUSED(resource);
    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    OutputChangesetPrivate::get(pendingChanges(output))->scaleFactor = scale;
}

void OutputConfigurationPrivate::outputconfiguration_apply(Resource *resource)
{
    Q_UNUSED(resource);

    Q_Q(OutputConfiguration);
    Q_EMIT q->changeRequested();
}

/*
 * OutputConfiguration
 */

OutputConfiguration::OutputConfiguration()
    : QWaylandCompositorExtensionTemplate<OutputConfiguration>(*new OutputConfigurationPrivate())
{
}

OutputConfiguration::OutputConfiguration(OutputManagement *parent, const QWaylandResource &resource)
    : QWaylandCompositorExtensionTemplate<OutputConfiguration>(*new OutputConfigurationPrivate())
{
    initialize(parent, resource);
}

OutputConfiguration::~OutputConfiguration()
{
    Q_D(OutputConfiguration);
    OutputManagementPrivate::get(d->management)->removeConfiguration(this);
}

void OutputConfiguration::initialize(OutputManagement *parent, const QWaylandResource &resource)
{
    Q_D(OutputConfiguration);
    d->management = parent;
    d->init(resource.resource());
    setExtensionContainer(parent);
    QWaylandCompositorExtension::initialize();
}

void OutputConfiguration::setApplied()
{
    Q_D(OutputConfiguration);
    d->clearPendingChanges();
    d->send_applied();
}

void OutputConfiguration::setFailed()
{
    Q_D(OutputConfiguration);
    d->clearPendingChanges();
    d->send_failed();
}

QHash<QWaylandOutput *, OutputChangeset *> OutputConfiguration::changes() const
{
    Q_D(const OutputConfiguration);
    return d->changes;
}

const struct wl_interface *OutputConfiguration::interface()
{
    return OutputConfigurationPrivate::interface();
}

QByteArray OutputConfiguration::interfaceName()
{
    return OutputConfigurationPrivate::interfaceName();
}

OutputConfiguration *OutputConfiguration::fromResource(wl_resource *resource)
{
    OutputConfigurationPrivate::Resource *res =
            OutputConfigurationPrivate::Resource::fromResource(resource);
    if (res)
        return static_cast<OutputConfigurationPrivate *>(res->outputconfiguration_object)->q_func();
    return Q_NULLPTR;
}

void OutputConfiguration::initialize()
{
    QWaylandCompositorExtension::initialize();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_outputconfiguration.cpp"
