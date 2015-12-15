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

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>

#include "clientconnection.h"
#include "clientconnection_p.h"

#include <wayland-client.h>

namespace GreenIsland {

namespace Client {

/*
 * ClientConnectionPrivate
 */

ClientConnectionPrivate::ClientConnectionPrivate()
    : display(Q_NULLPTR)
    , fd(-1)
{
}

ClientConnectionPrivate::~ClientConnectionPrivate()
{
    if (display) {
        wl_display_flush(display);
        wl_display_disconnect(display);
    }
}

void ClientConnectionPrivate::_q_initConnection()
{
    Q_Q(ClientConnection);

    // Try to connect to the server
    if (!display) {
        if (fd != -1)
            display = wl_display_connect_to_fd(fd);
        else
            display = wl_display_connect(socketName.toUtf8().constData());
    }
    if (!display) {
        Q_EMIT q->failed();
        return;
    }

    // Setup socket notifier and emit the signal
    setupSocketNotifier();
    Q_EMIT q->connected();
}

void ClientConnectionPrivate::setupSocketNotifier()
{
    Q_Q(ClientConnection);

    // Do not listen for events if we were given a display
    // instead of a fd or a socket name, we don't want to
    // interfere with qtwayland
    if (fd == -1 && socketName.isEmpty())
        return;

    const int fd = wl_display_get_fd(display);
    socketNotifier.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
    q->connect(socketNotifier.data(), &QSocketNotifier::activated, q, [q, this] {
        if (!display)
            return;
        wl_display_dispatch(display);
        Q_EMIT q->eventsDispatched();
    });
}

/*
 * ClientConnection
 */

ClientConnection::ClientConnection(QObject *parent)
    : QObject(*new ClientConnectionPrivate(), parent)
{
    connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this, [this] {
        flush();
    }, Qt::DirectConnection);
}

wl_display *ClientConnection::display() const
{
    Q_D(const ClientConnection);
    return d->display;
}

void ClientConnection::setDisplay(wl_display *display)
{
    Q_D(ClientConnection);

    if (!d->display)
        d->display = display;
}

int ClientConnection::socketFd() const
{
    Q_D(const ClientConnection);
    return d->fd;
}

void ClientConnection::setSocketFd(int fd)
{
    Q_D(ClientConnection);

    if (d->display)
        return;

    d->fd = fd;
}

QString ClientConnection::socketName() const
{
    Q_D(const ClientConnection);
    return d->socketName;
}

void ClientConnection::setSocketName(const QString &socketName)
{
    Q_D(ClientConnection);

    if (d->display)
        return;

    d->socketName = socketName;
}

void ClientConnection::initializeConnection()
{
    QMetaObject::invokeMethod(this, "_q_initConnection", Qt::QueuedConnection);
}

void ClientConnection::flush()
{
    Q_D(ClientConnection);

    if (!d->display)
        return;

    wl_display_flush(d->display);
}

} // namespace Client

} // namespace GreenIsland

#include "moc_clientconnection.cpp"
