/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QThread>

#include "waylandconnection.h"

WaylandConnection::WaylandConnection(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_connection(nullptr)
    , m_thread(new QThread(this))
{
}

WaylandConnection::~WaylandConnection()
{
    delete m_connection;

    m_thread->quit();
    m_thread->wait();
}

bool WaylandConnection::isConnected() const
{
    return m_connection && m_connection->isConnected();
}

QString WaylandConnection::socketName() const
{
    return m_socketName;
}

void WaylandConnection::setSocketName(const QString &socketName)
{
    if (m_socketName == socketName)
        return;

    if (m_initialized) {
        qWarning("Cannot set WaylandConnection::socketName after initialization");
        return;
    }

    m_socketName = socketName;
    Q_EMIT socketNameChanged();
}

void WaylandConnection::start()
{
    if (m_initialized && m_connection)
        m_connection->initializeConnection();
}

void WaylandConnection::flush()
{
    if (m_initialized && m_connection)
        m_connection->flush();
}

void WaylandConnection::componentComplete()
{
    m_initialized = true;

    if (m_socketName.isEmpty()) {
        //m_connection = ClientConnection::fromQt();
        m_connection = new ClientConnection();
    } else {
        m_connection = new ClientConnection();
        m_connection->setSocketName(m_socketName);
    }

    m_connection->moveToThread(m_thread);

    connect(m_connection, &ClientConnection::connected,
            this, &WaylandConnection::connected);
    connect(m_connection, &ClientConnection::failed,
            this, &WaylandConnection::failed);
    connect(m_connection, &ClientConnection::connected,
            this, &WaylandConnection::connectedChanged);
    connect(m_connection, &ClientConnection::failed,
            this, &WaylandConnection::connectedChanged);

    m_thread->start();
}

#include "moc_waylandconnection.cpp"
