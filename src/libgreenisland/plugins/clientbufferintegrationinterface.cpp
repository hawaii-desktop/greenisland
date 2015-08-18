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

#include "clientbufferintegrationinterface.h"

namespace GreenIsland {

/*
 * ClientBufferIntegrationInterfacePrivate
 */

class ClientBufferIntegrationInterfacePrivate
{
public:
    ClientBufferIntegrationInterfacePrivate()
    {
    }

    AbstractCompositor *compositor;
};

/*
 * ClientBufferIntegrationInterface
 */

ClientBufferIntegrationInterface::ClientBufferIntegrationInterface()
    : d_ptr(new ClientBufferIntegrationInterfacePrivate)
{
}

ClientBufferIntegrationInterface::~ClientBufferIntegrationInterface()
{
    delete d_ptr;
}

void ClientBufferIntegrationInterface::setCompositor(AbstractCompositor *compositor)
{
    Q_D(ClientBufferIntegrationInterface);
    d->compositor = compositor;
}

GLuint ClientBufferIntegrationInterface::textureForBuffer(wl_resource *buffer)
{
    Q_UNUSED(buffer);
    return 0;
}

void ClientBufferIntegrationInterface::destroyTextureForBuffer(wl_resource *buffer)
{
    Q_UNUSED(buffer);
}

bool ClientBufferIntegrationInterface::isYInverted(wl_resource *resource) const
{
    Q_UNUSED(resource);
    return true;
}

void *ClientBufferIntegrationInterface::lockNativeBuffer(wl_resource *resource) const
{
    Q_UNUSED(resource);
    return Q_NULLPTR;
}

void ClientBufferIntegrationInterface::unlockNativeBuffer(void *) const
{
}

QSize ClientBufferIntegrationInterface::bufferSize(wl_resource *resource) const
{
    Q_UNUSED(resource);
    return QSize();
}

} // namespace GreenIsland
