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
