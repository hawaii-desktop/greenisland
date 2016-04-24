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

#include "outputchangeset.h"
#include "outputchangeset_p.h"

namespace GreenIsland {

namespace Server {

/*
 * OutputChangesetPrivate
 */

OutputChangesetPrivate::OutputChangesetPrivate(QWaylandOutput *output)
    : output(output)
    // TODO: Implement enabled
    //, enabled(output->isEnabled())
    , enabled(true)
    , primary(output->compositor()->defaultOutput() == output)
    , transform(output->transform())
    , modeId(output->modes().indexOf(output->currentMode()))
    , position(output->position())
    , scaleFactor(output->scaleFactor())
{
}

/*
 * OutputChangeset
 */

OutputChangeset::OutputChangeset(QWaylandOutput *output, QObject *parent)
    : QObject(*new OutputChangesetPrivate(output), parent)
{
}

QWaylandOutput *OutputChangeset::output() const
{
    Q_D(const OutputChangeset);
    return d->output;
}

bool OutputChangeset::isEnabledChanged() const
{
    Q_D(const OutputChangeset);
    // TODO: Uncomment when enabled is implemented
    return false;
    //return d->enabled != d->output->isEnabled();
}

bool OutputChangeset::isPrimaryChanged() const
{
    Q_D(const OutputChangeset);
    return d->primary != (d->output->compositor()->defaultOutput() == d->output);
}

bool OutputChangeset::isTransformChanged() const
{
    Q_D(const OutputChangeset);
    return d->transform != d->output->transform();
}

bool OutputChangeset::isModeIdChanged() const
{
    Q_D(const OutputChangeset);
    return d->modeId != d->output->modes().indexOf(d->output->currentMode());
}

bool OutputChangeset::isPositionChanged() const
{
    Q_D(const OutputChangeset);
    return d->position != d->output->position();
}

bool OutputChangeset::isScaleFactorChanged() const
{
    Q_D(const OutputChangeset);
    return d->scaleFactor != d->output->scaleFactor();
}

bool OutputChangeset::isEnabled() const
{
    Q_D(const OutputChangeset);
    return d->enabled;
}

bool OutputChangeset::isPrimary() const
{
    Q_D(const OutputChangeset);
    return d->primary;
}

int OutputChangeset::modeId() const
{
    Q_D(const OutputChangeset);
    return d->modeId;
}

QWaylandOutput::Transform OutputChangeset::transform() const
{
    Q_D(const OutputChangeset);
    return d->transform;
}

QPoint OutputChangeset::position() const
{
    Q_D(const OutputChangeset);
    return d->position;
}

int OutputChangeset::scaleFactor() const
{
    Q_D(const OutputChangeset);
    return d->scaleFactor;
}

} // namespace Server

} // namespace GreenIsland

#include "moc_outputchangeset.cpp"
