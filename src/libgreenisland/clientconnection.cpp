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

#include <QtCore/private/qobject_p.h>

#include "clientconnection.h"

#include "qwlcompositor_p.h"

#include <wayland-server.h>
#include <wayland-util.h>

namespace GreenIsland {

/*
 * ClientConnectionPrivate
 */

class ClientConnectionPrivate : public QObjectPrivate
{
public:
    ClientConnectionPrivate(wl_client *_client)
        : client(_client)
    {
        // Save client credentials
        wl_client_get_credentials(client, &pid, &uid, &gid);
    }

    ~ClientConnectionPrivate()
    {
    }

    static void client_destroy_callback(wl_listener *listener, void *data)
    {
        Q_UNUSED(data);

        ClientConnection *client = reinterpret_cast<Wrapper *>(listener)->parent;
        Q_ASSERT(client != 0);
        WlCompositor::instance()->m_clients.removeOne(client);
        delete client;
    }

    wl_client *client;

    uid_t uid;
    gid_t gid;
    pid_t pid;

    struct Wrapper {
        wl_listener listener;
        ClientConnection *parent;
    };
    Wrapper listener;
};

/*
 * ClientConnection
 */

ClientConnection::ClientConnection(wl_client *client)
    : QObject(*new ClientConnectionPrivate(client))
{
    Q_D(ClientConnection);

    // Destroy wrapper when the client goes away
    d->listener.parent = this;
    d->listener.listener.notify = ClientConnectionPrivate::client_destroy_callback;
    wl_client_add_destroy_listener(client, &d->listener.listener);
}

ClientConnection::~ClientConnection()
{
    Q_D(ClientConnection);

    // Remove listener from signal
    wl_list_remove(&d->listener.listener.link);
}

wl_client *ClientConnection::client() const
{
    Q_D(const ClientConnection);
    return d->client;
}

qint64 ClientConnection::userId() const
{
    Q_D(const ClientConnection);
    return d->uid;
}

qint64 ClientConnection::groupId() const
{
    Q_D(const ClientConnection);
    return d->gid;
}

qint64 ClientConnection::processId() const
{
    Q_D(const ClientConnection);
    return d->pid;
}

void ClientConnection::kill(int sig)
{
    Q_D(ClientConnection);
    ::kill(d->pid, sig);
}

ClientConnection *ClientConnection::fromWlClient(wl_client *wlClient)
{
    if (!wlClient)
        return Q_NULLPTR;

    ClientConnection *client = Q_NULLPTR;

    wl_listener *l = wl_client_get_destroy_listener(wlClient,
        ClientConnectionPrivate::client_destroy_callback);
    if (l)
        client = reinterpret_cast<ClientConnectionPrivate::Wrapper *>(
            wl_container_of(l, (ClientConnectionPrivate::Wrapper *)0, listener))->parent;

    if (!client) {
        // The original idea was to create ClientConnection instances when
        // a client bound wl_compositor, but it's legal for a client to
        // bind several times resulting in multiple ClientConnection
        // instances for the same wl_client therefore we create it from
        // here on demand
        client = new ClientConnection(wlClient);
        WlCompositor::instance()->m_clients.append(client);
    }

    return client;
}

void ClientConnection::close()
{
    WlCompositor::instance()->destroyClient(this);
}

} // namespace GreenIsland

#include "moc_clientconnection.cpp"
