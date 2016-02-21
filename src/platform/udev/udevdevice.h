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
