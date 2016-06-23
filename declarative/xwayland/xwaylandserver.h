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

#ifndef XWAYLANDSERVER_H
#define XWAYLANDSERVER_H

#include <QtCore/QString>
#include <QtCore/QProcess>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>

struct wl_client;

class ServerProcess;

class XWaylandServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor CONSTANT)
public:
    XWaylandServer(QWaylandCompositor *compositor, QObject *parent = Q_NULLPTR);
    ~XWaylandServer();

    inline int wmFd() const {
        return m_wmFd;
    }

    inline wl_client *client() const {
        return m_client;
    }

    QWaylandCompositor *compositor() const;

    bool setup();

Q_SIGNALS:
    void spawned();
    void started();

private:
    QWaylandCompositor *m_compositor;

    int m_display;
    QString m_displayName;

    QString m_lockFileName;
    int m_abstractFd;
    int m_unixFd;
    int m_wmFd;

    pid_t m_pid;
    ServerProcess *m_process;

    wl_client *m_client;

    QString createLockFile(int display, int &status);

    int bindToAbstractSocket(int display);
    int bindToUnixSocket(int display);

    pid_t spawn();
    void shutdown();

private Q_SLOTS:
    void processEvents();
};

#endif // XWAYLANDSERVER_H
