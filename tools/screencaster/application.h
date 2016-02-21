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
