/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#include <QtCore/QThread>
#include <QtTest/QtTest>
#include <QtQuick/QQuickItem>

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/ClientConnection>
#include <GreenIsland/Client/Compositor>
#include <GreenIsland/Client/FullScreenShell>
#include <GreenIsland/Client/Output>
#include <GreenIsland/Client/Registry>
#include <GreenIsland/Client/Region>
#include <GreenIsland/Client/Screencaster>
#include <GreenIsland/Client/Screenshooter>
#include <GreenIsland/Client/Seat>
#include <GreenIsland/Client/Shm>
#include <GreenIsland/Client/ShmPool>
#include <GreenIsland/Client/Surface>
#include <GreenIsland/Server/Screencaster>
#include <GreenIsland/Server/Screenshooter>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>
#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>
#include <GreenIsland/QtWaylandCompositor/private/qwaylandcompositor_p.h>

using namespace GreenIsland;

static const QString s_socketName = QStringLiteral("greenisland-test-0");

class TestRegistry : public QObject
{
    Q_OBJECT
public:
    TestRegistry(QObject *parent = Q_NULLPTR)
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

        new QWaylandOutput(m_compositor, Q_NULLPTR);
        new Server::Screencaster(m_compositor);
        new Server::Screenshooter(m_compositor);

        m_compositor->create();
    }

    void cleanup()
    {
        delete m_compositor;
        m_compositor = Q_NULLPTR;
    }

    void testSetup()
    {
        Client::ClientConnection *display(new Client::ClientConnection());
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

        Client::Registry registry;
        registry.create(display->display());
        registry.setup();

        display->deleteLater();
        thread->quit();
        thread->wait();
        delete thread;
    }

#define TEST_CREATE_BEGIN() \
    Client::ClientConnection *display(new Client::ClientConnection()); \
    display->setSocketName(s_socketName); \
    \
    QThread *thread = new QThread(this); \
    display->moveToThread(thread); \
    thread->start(); \
    \
    QSignalSpy connectedSpy(display, SIGNAL(connected())); \
    QSignalSpy failedSpy(display, SIGNAL(failed())); \
    display->initializeConnection(); \
    QVERIFY(connectedSpy.wait()); \
    QCOMPARE(connectedSpy.count(), 1); \
    QCOMPARE(failedSpy.count(), 0); \
    QVERIFY(display->display())

#define TEST_CREATE_BODY(signalSignature, createMethod) \
    Client::Registry registry; \
    registry.create(display->display()); \
    QSignalSpy announced(&registry, signalSignature); \
    QVERIFY(announced.isValid()); \
    registry.setup(); \
    display->flushRequests(); \
    QVERIFY(announced.wait()); \
    const quint32 name = announced.first().first().value<quint32>(); \
    const quint32 version = announced.first().last().value<quint32>(); \
    auto *o = registry.createMethod(name, version, this); \
    QVERIFY(o); \
    o->deleteLater()

#define TEST_CREATE_END() \
    display->deleteLater(); \
    thread->quit(); \
    thread->wait(); \
    delete thread

    void testCompositor()
    {
        TEST_CREATE_BEGIN();

        TEST_CREATE_BODY(SIGNAL(compositorAnnounced(quint32,quint32)), createCompositor);

        Client::Surface *surface = o->createSurface(this);
        QVERIFY(surface);
        Client::Region *region = o->createRegion(this);
        QVERIFY(region);

        TEST_CREATE_END();

        delete surface;
        delete region;
    }

    /*
    void testFullScreenShell()
    {
        TEST_CREATE_BEGIN();
        TEST_CREATE_BODY(SIGNAL(fullScreenShellAnnounced(quint32,quint32)), createFullScreenShell);
        TEST_CREATE_END();
    }
    */

    void testOutput()
    {
        TEST_CREATE_BEGIN();
        TEST_CREATE_BODY(SIGNAL(outputAnnounced(quint32,quint32)), createOutput);
        TEST_CREATE_END();
    }

    void testSeat()
    {
        TEST_CREATE_BEGIN();

        TEST_CREATE_BODY(SIGNAL(seatAnnounced(quint32,quint32)), createSeat);

        QSignalSpy keyboardSpy(o, SIGNAL(keyboardAdded()));
        QVERIFY(keyboardSpy.isValid());
        QCOMPARE(keyboardSpy.count(), 0);

        QSignalSpy pointerSpy(o, SIGNAL(pointerAdded()));
        QVERIFY(pointerSpy.isValid());
        QCOMPARE(pointerSpy.count(), 0);

        QSignalSpy touchSpy(o, SIGNAL(touchAdded()));
        QVERIFY(touchSpy.isValid());
        QCOMPARE(touchSpy.count(), 0);

        new QWaylandInputDevice(m_compositor);

        display->flushRequests();
        QVERIFY(keyboardSpy.wait());
        //QVERIFY(pointerSpy.wait());
        //QVERIFY(touchSpy.wait());
        display->flushRequests();
        QCOMPARE(keyboardSpy.count(), 1);
        QCOMPARE(pointerSpy.count(), 1);
        QCOMPARE(touchSpy.count(), 1);

        TEST_CREATE_END();
    }

    void testShm()
    {
        TEST_CREATE_BEGIN();

        TEST_CREATE_BODY(SIGNAL(shmAnnounced(quint32,quint32)), createShm);

        Client::ShmPool *pool = o->createPool(1024);
        QVERIFY(pool);
        Client::BufferPtr buffer = pool->createBuffer(QSize(10, 10), 10 * 4);
        QVERIFY(!buffer.isNull());
        Client::BufferPtr buffer2 = pool->createBuffer(QSize(10, 10), 10 * 4, buffer.data()->address());
        QVERIFY(!buffer2.isNull());

        TEST_CREATE_END();

        delete pool;
    }

    void testScreencaster()
    {
        TEST_CREATE_BEGIN();

        Client::Registry registry;
        registry.create(display->display());

        QSignalSpy shmAnnounced(&registry, SIGNAL(shmAnnounced(quint32,quint32)));
        QSignalSpy announced(&registry, SIGNAL(screencasterAnnounced(quint32,quint32)));
        QVERIFY(shmAnnounced.isValid());
        QVERIFY(announced.isValid());

        registry.setup();
        display->flushRequests();

        QVERIFY(shmAnnounced.wait());
        //QVERIFY(announced.wait());

        const quint32 shmName = shmAnnounced.first().first().value<quint32>();
        const quint32 shmVersion = shmAnnounced.first().last().value<quint32>();
        Client::Shm *shm = registry.createShm(shmName, shmVersion, this);
        QVERIFY(shm);

        const quint32 name = announced.first().first().value<quint32>();
        const quint32 version = announced.first().last().value<quint32>();
        Client::Screencaster *screencaster = registry.createScreencaster(shm, name, version, this);
        QVERIFY(screencaster);

        delete screencaster;
        delete shm;

        TEST_CREATE_END();
    }

    void testScreenshooter()
    {
        TEST_CREATE_BEGIN();

        Client::Registry registry;
        registry.create(display->display());

        QSignalSpy shmAnnounced(&registry, SIGNAL(shmAnnounced(quint32,quint32)));
        QSignalSpy announced(&registry, SIGNAL(screenshooterAnnounced(quint32,quint32)));
        QVERIFY(shmAnnounced.isValid());
        QVERIFY(announced.isValid());

        registry.setup();
        display->flushRequests();

        QVERIFY(shmAnnounced.wait());
        //QVERIFY(announced.wait());

        const quint32 shmName = shmAnnounced.first().first().value<quint32>();
        const quint32 shmVersion = shmAnnounced.first().last().value<quint32>();
        Client::Shm *shm = registry.createShm(shmName, shmVersion, this);
        QVERIFY(shm);

        const quint32 name = announced.first().first().value<quint32>();
        const quint32 version = announced.first().last().value<quint32>();
        Client::Screenshooter *screenshooter = registry.createScreenshooter(shm, name, version, this);
        QVERIFY(screenshooter);

        delete screenshooter;
        delete shm;

        TEST_CREATE_END();
    }
};

QTEST_MAIN(TestRegistry)

#include "tst_registry.moc"
