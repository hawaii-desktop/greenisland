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

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/ClientConnection>
#include <GreenIsland/Client/Compositor>
#include <GreenIsland/Client/Registry>
#include <GreenIsland/Client/Shm>
#include <GreenIsland/Client/ShmPool>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>

using namespace GreenIsland;

static const QString s_socketName = QStringLiteral("greenisland-test-0");

class TestShmPool : public QObject
{
    Q_OBJECT
public:
    TestShmPool(QObject *parent = Q_NULLPTR)
        : QObject(parent)
        , m_compositor(Q_NULLPTR)
        , m_thread(Q_NULLPTR)
        , m_display(Q_NULLPTR)
        , m_shm(Q_NULLPTR)
        , m_shmPool(Q_NULLPTR)
    {
    }

private:
    QWaylandCompositor *m_compositor;
    QThread *m_thread;
    Client::ClientConnection *m_display;
    Client::Shm *m_shm;
    Client::ShmPool *m_shmPool;

private Q_SLOTS:
    void init()
    {
        delete m_compositor;
        m_compositor = new QWaylandCompositor(this);
        m_compositor->setSocketName(s_socketName.toUtf8());
        m_compositor->create();

        delete m_display;
        m_display = new Client::ClientConnection();
        m_display->setSocketName(s_socketName);

        delete m_thread;
        m_thread = new QThread(this);
        m_display->moveToThread(m_thread);
        m_thread->start();

        QSignalSpy connectedSpy(m_display, SIGNAL(connected()));
        QSignalSpy failedSpy(m_display, SIGNAL(failed()));
        m_display->initializeConnection();
        QVERIFY(connectedSpy.wait());
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(m_display->display());

        Client::Registry registry;
        registry.create(m_display->display());
        QSignalSpy shmAnnounced(&registry, SIGNAL(shmAnnounced(quint32,quint32)));
        QVERIFY(shmAnnounced.isValid());
        registry.setup();

        QVERIFY(shmAnnounced.wait());

        const quint32 shmName = shmAnnounced.first().first().value<quint32>();
        const quint32 shmVersion = shmAnnounced.first().last().value<quint32>();
        delete m_shm;
        m_shm = registry.createShm(shmName, shmVersion, this);
        QVERIFY(m_shm);

        delete m_shmPool;
        m_shmPool = m_shm->createPool(1024);
    }

    void cleanup()
    {
        delete m_shmPool;
        m_shmPool = Q_NULLPTR;

        delete m_shm;
        m_shm = Q_NULLPTR;

        delete m_compositor;
        m_compositor = Q_NULLPTR;

        if (m_thread) {
            m_thread->quit();
            m_thread->wait();
            delete m_thread;
            m_thread = Q_NULLPTR;
        }

        delete m_display;
        m_display = Q_NULLPTR;
    }

    void testCreateEmptyBuffer()
    {
        QSize size(0, 0);
        QVERIFY(size.isNull());
        QVERIFY(!m_shmPool->createBuffer(size, 0));
    }

    void testCreateBufferFromNullImage()
    {
        QImage image;
        QVERIFY(image.isNull());
        QVERIFY(!m_shmPool->createBuffer(image));
    }

    void testCreateBufferInvalidSize()
    {
        QSize size;
        QVERIFY(!size.isValid());
        QVERIFY(!m_shmPool->createBuffer(size, 0));
    }

    void testCreateBufferFromImage()
    {
        QImage image(42, 42, QImage::Format_RGB888);
        image.fill(Qt::red);
        QVERIFY(!image.isNull());

        Client::BufferSharedPtr buffer = m_shmPool->createBuffer(image).toStrongRef();
        QVERIFY(buffer);
        QCOMPARE(image.byteCount(), buffer->stride() * buffer->size().height());
        QCOMPARE(image.size(), buffer->size());

        QImage image2(buffer->address(), buffer->size().width(),
                      buffer->size().height(), QImage::Format_RGB888);
        QCOMPARE(image, image2);
    }

    void testCreateBufferFromData()
    {
        QImage image(42, 42, QImage::Format_RGB888);
        image.fill(Qt::red);
        QVERIFY(!image.isNull());

        Client::BufferSharedPtr buffer = m_shmPool->createBuffer(image.size(),
                                                                 image.bytesPerLine(), image.constBits()).toStrongRef();
        QVERIFY(buffer);
        QCOMPARE(image.byteCount(), buffer->stride() * buffer->size().height());
        QCOMPARE(image.size(), buffer->size());

        QImage image2(buffer->address(), buffer->size().width(),
                      buffer->size().height(), QImage::Format_RGB888);
        QCOMPARE(image, image2);
    }

    void testReuseBuffer()
    {
        QImage image(42, 42, QImage::Format_RGB888);
        image.fill(Qt::red);
        QVERIFY(!image.isNull());

        Client::BufferSharedPtr buffer = m_shmPool->createBuffer(image).toStrongRef();
        QVERIFY(buffer);
        buffer->setReleased(true);
        buffer->setUsed(false);

        // The second buffer should be the same because it's using the same image
        Client::BufferSharedPtr buffer2 = m_shmPool->createBuffer(image).toStrongRef();
        QCOMPARE(buffer, buffer2);
        buffer2->setReleased(true);
        buffer2->setUsed(false);

        // A bigger image should get a new buffer
        QImage image2(96, 96, QImage::Format_RGB888);
        image2.fill(Qt::red);
        QVERIFY(!image2.isNull());
        QVERIFY(image2 != image);
        Client::BufferSharedPtr buffer3 = m_shmPool->createBuffer(image2).toStrongRef();
        QVERIFY(buffer3 != buffer2);

        // An image with a different format should get a new buffer
        QImage image3(42, 42, QImage::Format_ARGB32);
        image3.fill(Qt::red);
        QVERIFY(!image3.isNull());
        QVERIFY(image3 != image);
        QVERIFY(image3 != image2);
        Client::BufferSharedPtr buffer4 = m_shmPool->createBuffer(image3).toStrongRef();
        QVERIFY(buffer4 != buffer2);
        QVERIFY(buffer4 != buffer3);
    }
};

QTEST_MAIN(TestShmPool)

#include "tst_shmpool.moc"
