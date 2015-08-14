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

#ifndef GREENISLAND_CLIENTCONNECTION_H
#define GREENISLAND_CLIENTCONNECTION_H

#include <QtCore/QObject>

#include <greenisland/greenisland_export.h>

extern "C" {
#include <signal.h>
}

struct wl_client;

namespace GreenIsland {

class ClientConnectionPrivate;

class GREENISLAND_EXPORT ClientConnection : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClientConnection)
    Q_PROPERTY(qint64 userId READ userId CONSTANT)
    Q_PROPERTY(qint64 groupId READ groupId CONSTANT)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)
public:
    virtual ~ClientConnection();

    wl_client *client() const;

    qint64 userId() const;
    qint64 groupId() const;

    qint64 processId() const;

    Q_INVOKABLE void kill(int sig = SIGTERM);

    static ClientConnection *fromWlClient(wl_client *wlClient);

public Q_SLOTS:
    void close();

private:
    explicit ClientConnection(wl_client *client);
};

} // namespace GreenIsland

Q_DECLARE_METATYPE(GreenIsland::ClientConnection*)

#endif // GREENISLAND_CLIENTCONNECTION_H
