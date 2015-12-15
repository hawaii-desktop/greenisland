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

#include <wayland-client.h>

namespace GreenIsland {

/*
 * WlClientConnectionPrivate
 */

class WlClientConnectionPrivate
{
public:
    WlClientConnectionPrivate(WlClientConnection *q)
        : display(Q_NULLPTR)
        , fd(-1)
        , q(q)
    {
    }

    ~WlClientConnectionPrivate()
    {
        if (display) {
            wl_display_flush(display);
            wl_display_disconnect(display);
        }
    }

    void _q_initConnection()
    {
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

    wl_display *display;
    int fd;
    QString socketName;
    QScopedPointer<QSocketNotifier> socketNotifier;

private:
    WlClientConnection *q;

    void setupSocketNotifier()
    {
        // Do not listen for events if we were given a display
        // instead of a fd or a socket name, we don't want to
        // interfere with qtwayland
        if (fd == -1 && socketName.isEmpty())
            return;

        const int fd = wl_display_get_fd(display);
        socketNotifier.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
        q->connect(socketNotifier.data(), &QSocketNotifier::activated, q, [this] {
            if (!display)
                return;
            wl_display_dispatch(display);
            Q_EMIT q->eventsDispatched();
        });
    }
};

/*
 * WlClientConnection
 */

WlClientConnection::WlClientConnection(QObject *parent)
    : QObject(parent)
    , d_ptr(new WlClientConnectionPrivate(this))
{
    connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this, [this] {
        flush();
    }, Qt::DirectConnection);
}

WlClientConnection::~WlClientConnection()
{
    delete d_ptr;
}

wl_display *WlClientConnection::display() const
{
    Q_D(const WlClientConnection);
    return d->display;
}

void WlClientConnection::setDisplay(wl_display *display)
{
    Q_D(WlClientConnection);

    if (!d->display)
        d->display = display;
}

int WlClientConnection::socketFd() const
{
    Q_D(const WlClientConnection);
    return d->fd;
}

void WlClientConnection::setSocketFd(int fd)
{
    Q_D(WlClientConnection);

    if (d->display)
        return;

    d->fd = fd;
}

QString WlClientConnection::socketName() const
{
    Q_D(const WlClientConnection);
    return d->socketName;
}

void WlClientConnection::setSocketName(const QString &socketName)
{
    Q_D(WlClientConnection);

    if (d->display)
        return;

    d->socketName = socketName;
}

void WlClientConnection::initializeConnection()
{
    QMetaObject::invokeMethod(this, "_q_initConnection", Qt::QueuedConnection);
}

void WlClientConnection::flush()
{
    Q_D(WlClientConnection);

    if (!d->display)
        return;

    wl_display_flush(d->display);
}

}

#include "moc_clientconnection.cpp"
