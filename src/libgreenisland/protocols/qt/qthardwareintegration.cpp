/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qthardwareintegration.h"

namespace GreenIsland {

QtHardwareIntegration::QtHardwareIntegration(wl_display *display)
    : qt_hardware_integration(display, 1)
{
}

void QtHardwareIntegration::setClientBufferIntegration(const QString &name)
{
    m_clientBufferIntegration = name;
}

void QtHardwareIntegration::setServerBufferIntegration(const QString &name)
{
    m_serverBufferIntegration = name;
}

void QtHardwareIntegration::hardware_integration_bind_resource(Resource *resource)
{
    // Send buffer integrations name when clients bind this interface
    if (!m_clientBufferIntegration.isEmpty())
        send_client_backend(resource->handle, m_clientBufferIntegration);
    if (!m_serverBufferIntegration.isEmpty())
        send_server_backend(resource->handle, m_serverBufferIntegration);
}

} // namespace GreenIsland

