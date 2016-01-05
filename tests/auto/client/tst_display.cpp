/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#include <QtCore/QThread>
#include <QtTest/QtTest>

#include <GreenIsland/Client/ClientConnection>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/private/qwaylandcompositor_p.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace GreenIsland::Client;

static const QString s_socketName = QStringLiteral("greenisland-test-0");

class TestDisplay : public QObject
{
    Q_OBJECT
public:
    TestDisplay(QObject *parent = Q_NULLPTR)
        : QObject(parent)
        , m_compositor(Q_NULLPTR)
    {
    }

private:
    QWaylandCompositor *m_compositor;

private Q_SLOTS:
    void init()
    {
        delete m_compositor;
        m_compositor = new QWaylandCompositor(this);
        m_compositor->setSocketName(s_socketName.toUtf8());
        m_compositor->create();
    }

    void cleanup()
    {
        delete m_compositor;
        m_compositor = Q_NULLPTR;
    }

    void testNoThread()
    {
        QScopedPointer<ClientConnection> display(new ClientConnection());

        // Socket name is wayland-0 by default
        QCOMPARE(display->socketName(), QStringLiteral("wayland-0"));

        // Test socket name change
        display->setSocketName(s_socketName);
        QCOMPARE(display->socketName(), s_socketName);

        // Test connection
        QSignalSpy connectedSpy(display.data(), SIGNAL(connected()));
        QSignalSpy failedSpy(display.data(), SIGNAL(failed()));
        display->initializeConnection();
        QVERIFY(connectedSpy.wait());
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(display->display());
    }

    void testFailure()
    {
        QScopedPointer<ClientConnection> display(new ClientConnection());
        display->setSocketName(QStringLiteral("greenisland-test-bad-socket-name"));

        QSignalSpy connectedSpy(display.data(), SIGNAL(connected()));
        QSignalSpy failedSpy(display.data(), SIGNAL(failed()));
        display->initializeConnection();
        QVERIFY(failedSpy.wait());
        QCOMPARE(connectedSpy.count(), 0);
        QCOMPARE(failedSpy.count(), 1);
        QVERIFY(!display->display());
    }

    void testWithThread()
    {
        ClientConnection *display(new ClientConnection());
        display->setSocketName(s_socketName);

        QThread *thread = new QThread(this);
        display->moveToThread(thread);
        thread->start();

        QSignalSpy connectedSpy(display, SIGNAL(connected()));
        QSignalSpy failedSpy(display, SIGNAL(failed()));
        display->initializeConnection();
        QVERIFY(connectedSpy.wait());
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(display->display());

        display->deleteLater();
        thread->quit();
        thread->wait();
        delete thread;
    }

    void testSocketFd()
    {
        int sv[2];
        QVERIFY(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) >= 0);

        ClientConnection *display(new ClientConnection());
        display->setSocketFd(sv[1]);

        QThread *thread = new QThread(this);
        display->moveToThread(thread);
        thread->start();

        QSignalSpy connectedSpy(display, SIGNAL(connected()));
        QSignalSpy failedSpy(display, SIGNAL(failed()));
        display->initializeConnection();
        QVERIFY(connectedSpy.wait());
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(display->display());

        display->deleteLater();
        thread->quit();
        thread->wait();
        delete thread;
    }
};

QTEST_MAIN(TestDisplay)

#include "tst_display.moc"
