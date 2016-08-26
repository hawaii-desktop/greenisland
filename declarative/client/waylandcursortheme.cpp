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

#include <GreenIsland/Client/Output>

#include "waylandconnection.h"
#include "waylandcursortheme.h"

WaylandCursorTheme::WaylandCursorTheme(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_connection(nullptr)
    , m_registry(new Registry())
    , m_compositor(nullptr)
    , m_shm(nullptr)
    , m_shmPool(nullptr)
    , m_outputsDone(0)
    , m_seat(nullptr)
    , m_cursorTheme(nullptr)
    , m_cursorShape(WaylandCursorTheme::CursorShape::ArrowCursor)
{
    connect(m_registry, &Registry::interfaceAnnounced,
            this, &WaylandCursorTheme::interfaceAnnounced);
    connect(m_registry, &Registry::interfacesAnnounced,
            this, &WaylandCursorTheme::interfacesAnnounced);
    connect(m_registry, &Registry::interfacesRemoved,
            this, &WaylandCursorTheme::interfacesRemoved);
}

WaylandCursorTheme::~WaylandCursorTheme()
{
    delete m_registry;
}

WaylandConnection *WaylandCursorTheme::connection() const
{
    return m_connection;
}

void WaylandCursorTheme::setConnection(WaylandConnection *connection)
{
    if (m_connection == connection)
        return;

    if (m_initialized) {
        qWarning("Cannot set WaylandCursorTheme::connection after initialization");
        return;
    }

    if (!connection)
        disconnect(m_connection, &WaylandConnection::connected,
                   this, &WaylandCursorTheme::handleConnected);

    m_connection = connection;
    Q_EMIT connectionChanged();

    if (m_connection)
        m_initialized = true;

    if (m_connection && m_connection->isConnected())
        handleConnected();
    else if (m_connection)
        connect(m_connection, &WaylandConnection::connected,
                this, &WaylandCursorTheme::handleConnected,
                Qt::QueuedConnection);
}

WaylandCursorTheme::CursorShape WaylandCursorTheme::shape() const
{
    return m_cursorShape;
}

void WaylandCursorTheme::setShape(CursorShape shape)
{
    if (m_cursorShape == shape)
        return;

    if (m_cursorTheme) {
        m_cursorTheme->changeCursor(static_cast<CursorTheme::CursorShape>(shape));
        m_cursorShape = shape;
        Q_EMIT shapeChanged();
    }
}

void WaylandCursorTheme::handleConnected()
{
    m_registry->create(m_connection->display());
    m_registry->setup();
}

void WaylandCursorTheme::interfaceAnnounced(const QByteArray &interface, quint32 name, quint32 version)
{
    if (interface == QByteArray("wl_compositor"))
        m_compositor = m_registry->createCompositor(name, version, this);
    else if (interface == QByteArray("wl_shm"))
        m_shm = m_registry->createShm(name, version, this);
    else if (interface == QByteArray("wl_seat"))
        m_seat = m_registry->createSeat(name, version, this);
    else if (interface == QByteArray("wl_output")) {
        Output *output = m_registry->createOutput(name, version, this);
        connect(output, &Output::outputChanged, this, [this] {
            m_outputsDone++;

            if (m_outputsDone == m_outputs.count())
                interfacesAnnounced();
        });
        m_outputs.append(output);
    }
}

void WaylandCursorTheme::interfacesAnnounced()
{
    if (m_cursorTheme)
        return;

    if (m_compositor && m_shm && m_seat && m_outputsDone == m_outputs.count()) {
        m_shmPool = m_shm->createPool(1024);
        m_cursorTheme = new CursorTheme(m_registry, m_compositor, m_shmPool, m_seat);
        m_cursorTheme->changeCursor(static_cast<CursorTheme::CursorShape>(m_cursorShape));
    }
}

void WaylandCursorTheme::interfacesRemoved()
{
    delete m_cursorTheme;
    m_cursorTheme = nullptr;

    delete m_shmPool;
    m_shmPool = nullptr;

    delete m_shm;
    m_shm = nullptr;

    qDeleteAll(m_outputs);

    delete m_seat;
    m_seat = nullptr;

    delete m_compositor;
    m_compositor = nullptr;
}
