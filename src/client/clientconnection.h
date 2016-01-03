/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLANDCLIENT_CLIENTCONNECTION_H
#define GREENISLANDCLIENT_CLIENTCONNECTION_H

#include <QtCore/QObject>

#include <GreenIsland/client/greenislandclient_export.h>

struct wl_display;

namespace GreenIsland {

namespace Client {

class ClientConnectionPrivate;

class GREENISLANDCLIENT_EXPORT ClientConnection : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClientConnection)
public:
    ClientConnection(QObject *parent = Q_NULLPTR);

    bool isConnected() const;

    wl_display *display() const;
    void setDisplay(wl_display *display);

    int socketFd() const;
    void setSocketFd(int fd);

    QString socketName() const;
    void setSocketName(const QString &socketName);

    void initializeConnection();
    void synchronousConnection();

    static ClientConnection *fromQt(QObject *parent = Q_NULLPTR);

public Q_SLOTS:
    void forceRoundTrip();
    void flushRequests();
    void blockingReadEvents();

Q_SIGNALS:
    void connected();
    void failed();
    void eventsDispatched();

private:
    Q_PRIVATE_SLOT(d_func(), void _q_initConnection())
};

} // namespace Client

} // namespace GreenIslanmd

#endif // GREENISLANDCLIENT_CLIENTCONNECTION_H
