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

#include <QtTest/QtTest>

#include <GreenIsland/Platform/Udev>
#include <GreenIsland/Platform/UdevEnumerate>

using namespace GreenIsland::Platform;

class TestUdev : public QObject
{
    Q_OBJECT
public:
    TestUdev(QObject *parent = 0)
        : QObject(parent)
    {
    }

private Q_SLOTS:
    void testConnection()
    {
        Udev *udev = new Udev;
        QVERIFY(udev->isValid());
        delete udev;
    }

    void testDevice()
    {
        Udev *udev = new Udev;
        QVERIFY(udev->isValid());

        UdevDevice *dev = udev->deviceFromFileName(QStringLiteral("/dev/sda"));
        QCOMPARE(dev->deviceNode(), QStringLiteral("/dev/sda"));
        QVERIFY(dev);

        delete dev;
        delete udev;
    }

    void testEnumerate()
    {
        Udev *udev = new Udev;
        QVERIFY(udev->isValid());

        UdevEnumerate *enumerate = new UdevEnumerate(UdevDevice::InputDevice_Mask, udev);
        QList<UdevDevice *> devices = enumerate->scan();
        QVERIFY(devices.size() > 0);

        delete enumerate;
        delete udev;
    }
};

QTEST_MAIN(TestUdev)

#include "tst_udev.moc"
