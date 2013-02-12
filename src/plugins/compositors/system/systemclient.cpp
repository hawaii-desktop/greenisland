/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "systemclient.h"

SystemClient::SystemClient(wl_client *client)
    : m_client(client)
    , m_resource(0)
    , m_readySent(false)
{
}

wl_client *SystemClient::client() const
{
    return m_client;
}

wl_resource *SystemClient::clientResource() const
{
    return m_resource;
}

void SystemClient::setClientResource(wl_resource *res)
{
    m_resource = res;
}

void SystemClient::unbindClientResource()
{
    free(m_resource);
    m_resource = 0;
}

QWaylandSurface *SystemClient::surfaceForOutput(QtWayland::Output *output) const
{
    for (int i = 0; i < m_mappings.size(); i++) {
        OutputSurfaceMapping mapping = m_mappings.at(i);

        if (mapping.first == output)
            return mapping.second;
    }

    return 0;
}

void SystemClient::mapSurfaceToOutput(QWaylandSurface *surface, QtWayland::Output *output)
{
    OutputSurfaceMapping mapping;
    mapping.first = output;
    mapping.second = surface;

    m_mappings.append(mapping);
}

WaylandSurfaceList SystemClient::surfaces() const
{
    WaylandSurfaceList list;

    for (int i = 0; i < m_mappings.size(); i++) {
        OutputSurfaceMapping mapping = m_mappings.at(i);
        list.append(mapping.second);
    }

    return list;
}

bool SystemClient::isReadySent() const
{
    return m_readySent;
}

void SystemClient::setReady()
{
    m_readySent = true;
}
