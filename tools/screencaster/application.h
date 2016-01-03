/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtCore/QObject>
#include <QtCore/QThread>

#include <GreenIsland/Client/ClientConnection>
#include <GreenIsland/Client/Registry>
#include <GreenIsland/Client/Screencaster>
#include <GreenIsland/Client/Shm>

using namespace GreenIsland;

class Recorder;

class StartupEvent : public QEvent
{
public:
    StartupEvent();
};

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(const QString &fileName, int maxFrames,
                         bool stillImages = false,
                         QObject *parent = Q_NULLPTR);
    ~Application();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    bool m_initialized;
    int m_maxFrames, m_curFrame;
    QThread *m_thread;
    Client::ClientConnection *m_connection;
    Client::Registry *m_registry;
    Client::Shm *m_shm;
    Client::Screencaster *m_screencaster;
    Recorder *m_recorder;

    void initialize();

private Q_SLOTS:
    void interfacesAnnounced();
    void interfaceAnnounced(const QByteArray &interface, quint32 name, quint32 version);
};

#endif // APPLICATION_H
