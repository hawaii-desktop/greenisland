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

#include <QtTest/QtTest>

#include <GreenIsland/Platform/Logind>

#include "fakelogind.h"

using namespace GreenIsland::Platform;

class CustomLogind : public Logind
{
public:
    CustomLogind(QObject *parent = 0)
        : Logind(QDBusConnection::sessionBus(), parent)
    {
    }
};

class TestLogind : public QObject
{
    Q_OBJECT
public:
    TestLogind(QObject *parent = 0)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testConnection()
    {
        // Integration is initially disconnected
        CustomLogind *logind = new CustomLogind;
        QVERIFY(!logind->isConnected());

        // Spy on connectedChanged(bool) and create the service
        QSignalSpy spyConnected(logind, SIGNAL(connectedChanged(bool)));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spyConnected.wait());
        QVERIFY(logind->isConnected());

        // Delete
        fakeLogind->deleteLater();
        QVERIFY(spyConnected.wait());
        QVERIFY(!logind->isConnected());
        logind->deleteLater();
    }

    void testRegistration()
    {
        // Integration is initially disconnected
        CustomLogind *logind = new CustomLogind;
        QVERIFY(!logind->isConnected());

        // Spy on connectedChanged(bool) and create the service
        QSignalSpy spyConnected(logind, SIGNAL(connectedChanged(bool)));
        QScopedPointer<FakeLogind> fakeLogind(new FakeLogind);
        QVERIFY(spyConnected.wait());
        QVERIFY(logind->isConnected());
        spyConnected.clear();

        // Now the service goes away
        fakeLogind.reset();

        // And the integration should no longer be connected
        QVERIFY(spyConnected.wait());
        QVERIFY(!logind->isConnected());
        spyConnected.clear();

        // Now the service is brought up again
        fakeLogind.reset(new FakeLogind);

        // And the integration should connect again
        QVERIFY(spyConnected.wait());
        QVERIFY(logind->isConnected());

        // Delete
        fakeLogind.reset();
        logind->deleteLater();
    }

    void testPropertySessionActive()
    {
        CustomLogind *logind = new CustomLogind;

        // Spy on sessionActiveChanged(bool)
        QSignalSpy spySessionActive(logind, SIGNAL(sessionActiveChanged(bool)));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spySessionActive.wait());
        QVERIFY(logind->isSessionActive());

        fakeLogind->deleteLater();
        logind->deleteLater();

        QTest::qWait(1000);
    }

    void testPropertyVtNumber()
    {
        CustomLogind *logind = new CustomLogind;
        QCOMPARE(logind->vtNumber(), -1);

        // Spy on vtNumberChanged(int)
        QSignalSpy spyVtNumber(logind, SIGNAL(vtNumberChanged(int)));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spyVtNumber.wait());
        QCOMPARE(logind->vtNumber(), 1);

        fakeLogind->deleteLater();
        logind->deleteLater();

        QTest::qWait(1000);
    }

    void testSessionControl()
    {
        // Connect to a fake logind service and wait for the signal
        CustomLogind *logind = new CustomLogind;
        QSignalSpy spyConnected(logind, SIGNAL(connectedChanged(bool)));
        QSignalSpy spyTakeControl(logind, SIGNAL(hasSessionControlChanged(bool)));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spyConnected.wait());

        // Take control as soon as we are connected to logind and
        // then release control, we should have received two signals
        logind->takeControl();
        spyTakeControl.wait();
        logind->releaseControl();
        spyTakeControl.wait();
        QCOMPARE(spyTakeControl.count(), 2);

        fakeLogind->deleteLater();
        logind->deleteLater();

        QTest::qWait(1000);
    }

    void testLockUnlock()
    {
        // Connect and spy on lockSessionRequested() and unlockSessionRequested()
        CustomLogind *logind = new CustomLogind;
        QSignalSpy spyConnected(logind, SIGNAL(connectedChanged(bool)));
        QSignalSpy spyLock(logind, SIGNAL(lockSessionRequested()));
        QSignalSpy spyUnlock(logind, SIGNAL(unlockSessionRequested()));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spyConnected.wait());

        // Lock
        fakeLogind->doLock();
        QVERIFY(spyLock.wait());

        // Unlock
        fakeLogind->doUnlock();
        QVERIFY(spyUnlock.wait());

        logind->deleteLater();
        fakeLogind->deleteLater();

        QTest::qWait(1000);
    }

    void testPrepareForSleep()
    {
        // Connect and spy on prepareForSleep(bool)
        CustomLogind *logind = new CustomLogind;
        QSignalSpy spyConnected(logind, SIGNAL(connectedChanged(bool)));
        QSignalSpy spyPrepare(logind, SIGNAL(prepareForSleep(bool)));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spyConnected.wait());

        // Prepare for sleep before
        fakeLogind->doPrepareForSleep(true);
        QVERIFY(spyPrepare.wait());

        // Prepare for sleep after
        fakeLogind->doPrepareForSleep(false);
        QVERIFY(spyPrepare.wait());

        logind->deleteLater();
        fakeLogind->deleteLater();

        QTest::qWait(1000);
    }

    void testPrepareForShutdown()
    {
        // Connect and spy on prepareForShutdown(bool)
        CustomLogind *logind = new CustomLogind;
        QSignalSpy spyConnected(logind, SIGNAL(connectedChanged(bool)));
        QSignalSpy spyPrepare(logind, SIGNAL(prepareForShutdown(bool)));
        FakeLogind *fakeLogind = new FakeLogind;
        QVERIFY(spyConnected.wait());

        // Prepare for shutdown before
        fakeLogind->doPrepareForShutdown(true);
        QVERIFY(spyPrepare.wait());

        // Prepare for shutdown after
        fakeLogind->doPrepareForShutdown(false);
        QVERIFY(spyPrepare.wait());

        logind->deleteLater();
        fakeLogind->deleteLater();

        QTest::qWait(1000);
    }
};

QTEST_MAIN(TestLogind)

#include "tst_logind.moc"
