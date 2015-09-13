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

#include "logging.h"
#include "screenbackend.h"

Q_LOGGING_CATEGORY(SCREEN_BACKEND, "greenisland.screenbackend")

namespace GreenIsland {

ScreenBackend::ScreenBackend(Compositor *compositor, QObject *parent)
    : QObject(parent)
    , m_compositor(compositor)
{
}

ScreenBackend::~ScreenBackend()
{
    qCDebug(SCREEN_BACKEND) << "Removing all outputs...";

    Q_FOREACH (Output *output, m_outputs) {
        if (m_outputs.removeOne(output)) {
            output->window()->close();
            delete output;
        }
    }
}

Compositor *ScreenBackend::compositor() const
{
    return m_compositor;
}

QList<Output *> ScreenBackend::outputs() const
{
    return m_outputs;
}

}

#include "moc_screenbackend.cpp"
