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

#ifndef GREENISLAND_CLIENTBUFFERINTEGRATIONPLUGIN_H
#define GREENISLAND_CLIENTBUFFERINTEGRATIONPLUGIN_H

#include <QtCore/QObject>

#include <GreenIsland/ClientBufferIntegrationInterface>

namespace GreenIsland {

class GREENISLAND_EXPORT ClientBufferIntegrationPlugin : public QObject, public ClientBufferIntegrationInterface
{
    Q_OBJECT
    Q_INTERFACES(ClientBufferIntegrationInterface)
public:
    explicit ClientBufferIntegrationPlugin(QObject *parent = 0);
    ~ClientBufferIntegrationPlugin();

    virtual void initializeHardware(Display *waylandDisplay) = 0;
    virtual void bindTextureToBuffer(wl_resource *buffer) = 0;

    virtual ClientBufferIntegrationInterface *create(const QString &key, const QStringList &args) = 0;

private:
    Q_DISABLE_COPY(ClientBufferIntegrationPlugin)
};

} // namespace GreenIsland

#endif // GREENISLAND_CLIENTBUFFERINTEGRATIONPLUGIN_H
