/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtGui/QGuiApplication>
#include <QtGui/qpa/qplatformnativeinterface.h>

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
    , displayFromQt(false)
    , fd(-1)
    , socketName(QString::fromUtf8(qgetenv("WAYLAND_DISPLAY")))
{
    if (socketName.isEmpty())
        socketName = QLatin1String("wayland-0");
}

ClientConnectionPrivate::~ClientConnectionPrivate()
{
    if (display && !displayFromQt) {
        wl_display_flush(display);
        wl_display_disconnect(display);
    }
}

void ClientConnectionPrivate::checkError()
{
    int error = wl_display_get_error(display);
    if (error == EPIPE || error == ECONNRESET)
        qWarning("The Wayland connection was interrupted");
    else
        qErrnoWarning(error, "The Wayland connection experienced a fatal error");
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
    connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock,
            this, &ClientConnection::flushRequests,
            Qt::DirectConnection);
    connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::awake,
            this, &ClientConnection::flushRequests,
            Qt::DirectConnection);
}

bool ClientConnection::isConnected() const
{
    Q_D(const ClientConnection);
    return d->display != Q_NULLPTR;
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
    Q_D(ClientConnection);

    if (d->displayFromQt)
        return;
    QMetaObject::invokeMethod(this, "_q_initConnection", Qt::QueuedConnection);
}

void ClientConnection::synchronousConnection()
{
    Q_D(ClientConnection);

    if (d->displayFromQt)
        return;
    d->_q_initConnection();
}

ClientConnection *ClientConnection::fromQt(QObject *parent)
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland")))
        return Q_NULLPTR;

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (!native)
        return Q_NULLPTR;

    wl_display *display = reinterpret_cast<wl_display *>(
                native->nativeResourceForIntegration(QByteArrayLiteral("wl_display")));
    if (!display)
        return Q_NULLPTR;

    ClientConnection *connection = new ClientConnection(parent);
    ClientConnectionPrivate::get(connection)->display = display;
    ClientConnectionPrivate::get(connection)->displayFromQt = true;
    disconnect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::awake,
               connection, &ClientConnection::flushRequests);
    return connection;
}

void ClientConnection::forceRoundTrip()
{
    Q_D(ClientConnection);

    if (!d->display)
        return;

    wl_display_roundtrip(d->display);
}

void ClientConnection::flushRequests()
{
    Q_D(ClientConnection);

    if (!d->display)
        return;

    if (!d->displayFromQt) {
        if (wl_display_prepare_read(d->display) == 0)
            wl_display_read_events(d->display);

        if (wl_display_dispatch_pending(d->display) < 0) {
            d->checkError();
            ::exit(1);
        }
    }

    wl_display_flush(d->display);
}

void ClientConnection::blockingReadEvents()
{
    Q_D(ClientConnection);

    if (!d->display)
        return;

    if (wl_display_dispatch(d->display) < 0) {
        d->checkError();
        ::exit(1);
    }
}

} // namespace Client

} // namespace GreenIsland

#include "moc_clientconnection.cpp"
