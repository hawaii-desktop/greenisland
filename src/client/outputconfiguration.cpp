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

#include "output_p.h"
#include "outputconfiguration.h"
#include "outputconfiguration_p.h"

namespace GreenIsland {

namespace Client {

/*
 * OutputConfigurationPrivate
 */

OutputConfigurationPrivate::OutputConfigurationPrivate()
    : QtWayland::greenisland_outputconfiguration()
{
}

/*
 * OutputConfiguration
 */

OutputConfiguration::OutputConfiguration(QObject *parent)
    : QObject(*new OutputConfigurationPrivate(), parent)
{
}

void OutputConfiguration::setEnabled(Output *output, bool value)
{
    Q_D(OutputConfiguration);

    auto wlOutput = OutputPrivate::get(output)->object();
    d->enable(wlOutput, value ? 1 : 0);
}

void OutputConfiguration::setPrimary(Output *output, bool value)
{
    Q_D(OutputConfiguration);

    auto wlOutput = OutputPrivate::get(output)->object();
    d->primary(wlOutput, value ? 1 : 0);
}

void OutputConfiguration::setModeId(Output *output, int modeId)
{
    Q_D(OutputConfiguration);

    auto wlOutput = OutputPrivate::get(output)->object();
    d->mode(wlOutput, modeId);
}

void OutputConfiguration::setTransform(Output *output, Output::Transform transform)
{
    Q_D(OutputConfiguration);

    // TODO: Convert transform to wltransform
    auto wlOutput = OutputPrivate::get(output)->object();
    d->transform(wlOutput, int32_t(transform));
}

void OutputConfiguration::setPosition(Output *output, const QPoint &position)
{
    Q_D(OutputConfiguration);

    auto wlOutput = OutputPrivate::get(output)->object();
    d->position(wlOutput, position.x(), position.y());
}

void OutputConfiguration::setScaleFactor(Output *output, qint32 scaleFactor)
{
    Q_D(OutputConfiguration);

    auto wlOutput = OutputPrivate::get(output)->object();
    d->scale(wlOutput, scaleFactor);
}

void OutputConfiguration::apply()
{
    Q_D(OutputConfiguration);
    d->apply();
}

} // namespace Client

} // namespace GreenIsland

#include "moc_outputconfiguration.cpp"
