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
