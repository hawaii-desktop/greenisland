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

#ifndef GREENISLAND_UDEVDEVICE_H
#define GREENISLAND_UDEVDEVICE_H

#include <QtCore/QObject>

#include <GreenIsland/platform/greenislandplatform_export.h>

struct udev_device;

namespace GreenIsland {

namespace Platform {

class Udev;
class UdevEnumerate;
class UdevDevicePrivate;
class UdevMonitorPrivate;

class GREENISLANDPLATFORM_EXPORT UdevDevice
{
    Q_DECLARE_PRIVATE(UdevDevice)
    Q_ENUMS(DeviceType)
public:
    enum DeviceType {
        UnknownDevice = 0x00,
        KeyboardDevice = 0x01,
        MouseDevice = 0x02,
        TouchpadDevice = 0x04,
        TouchscreenDevice = 0x08,
        TabletDevice = 0x10,
        JoystickDevice = 0x20,
        GenericVideoDevice = 0x40,
        PrimaryVideoDevice = 0x80,
        InputDevice_Mask = KeyboardDevice | MouseDevice | TouchpadDevice | TouchscreenDevice | TabletDevice | JoystickDevice,
        VideoDevice_Mask = GenericVideoDevice
    };
    Q_DECLARE_FLAGS(DeviceTypes, DeviceType)

    ~UdevDevice();

    DeviceTypes type() const;

    QString subsystem() const;
    QString devType() const;
    QString name() const;
    QString driver() const;

    QString deviceNode() const;
    QStringList alternateDeviceSymlinks() const;

    QString sysfsPath() const;
    int sysfsNumber() const;

    QStringList deviceProperties() const;
    QStringList sysfsProperties() const;

    UdevDevice *parent() const;

private:
    UdevDevice(udev_device *dev);

    UdevDevicePrivate *const d_ptr;

    friend class Udev;
    friend class UdevEnumerate;
    friend class UdevMonitorPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UdevDevice::DeviceTypes)

QDebug operator<<(QDebug, const UdevDevice &);

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_UDEVDEVICE_H
