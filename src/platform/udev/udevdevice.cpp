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

#include <QtCore/QDebug>

#include "udev/udev_p.h"
#include "udev/udevdevice.h"

namespace GreenIsland {

namespace Platform {

static inline QStringList listFromEntries(udev_list_entry *l)
{
    QStringList list;
    udev_list_entry *entry;

    udev_list_entry_foreach(entry, l) {
        list.append(QString::fromUtf8(udev_list_entry_get_name(entry)));
    }

    return list;
}

/*
 * UdevDevicePrivate
 */

class UdevDevicePrivate
{
public:
    UdevDevicePrivate()
        : device(Q_NULLPTR)
    {
    }

    ~UdevDevicePrivate()
    {
        if (device)
            udev_device_unref(device);
    }

    udev_device *device;
};

/*
 * UdevDevice
 */

UdevDevice::UdevDevice(udev_device *device)
    : d_ptr(new UdevDevicePrivate)
{
    Q_D(UdevDevice);
    d->device = device;
}

UdevDevice::~UdevDevice()
{
    delete d_ptr;
}

UdevDevice::DeviceTypes UdevDevice::type() const
{
    Q_D(const UdevDevice);

    DeviceTypes result = 0;

    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_KEYBOARD"), "1") == 0)
        result |= KeyboardDevice;
    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_KEY"), "1") == 0)
        result |= KeyboardDevice;

    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_MOUSE"), "1") == 0)
        result |= MouseDevice;

    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_TOUCHPAD"), "1") == 0)
        result |= TouchpadDevice;

    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_TOUCHSCREEN"), "1") == 0)
        result |= TouchscreenDevice;

    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_TABLET"), "1") == 0)
        result |= TabletDevice;

    if (qstrcmp(udev_device_get_property_value(d->device, "ID_INPUT_JOYSTICK"), "1") == 0)
        result |= JoystickDevice;

    if (qstrcmp(udev_device_get_subsystem(d->device), "drm") == 0) {
        bool isSet = false;

        udev_device *pci =
                udev_device_get_parent_with_subsystem_devtype(d->device, "pci", Q_NULLPTR);
        if (pci) {
            if (qstrcmp(udev_device_get_sysattr_value(pci, "boot_vga"), "1") == 0) {
                result |= PrimaryVideoDevice;
                isSet = true;
            }
        }

        if (!isSet)
            result |= GenericVideoDevice;
    }

    return result;
}

QString UdevDevice::subsystem() const
{
    Q_D(const UdevDevice);
    return QString::fromUtf8(udev_device_get_subsystem(d->device));
}

QString UdevDevice::devType() const
{
    Q_D(const UdevDevice);
    return QString::fromUtf8(udev_device_get_devtype(d->device));
}

QString UdevDevice::name() const
{
    Q_D(const UdevDevice);
    return QString::fromUtf8(udev_device_get_sysname(d->device));
}

QString UdevDevice::driver() const
{
    Q_D(const UdevDevice);
    return QString::fromUtf8(udev_device_get_driver(d->device));
}

QString UdevDevice::deviceNode() const
{
    Q_D(const UdevDevice);
    return QString::fromUtf8(udev_device_get_devnode(d->device));
}

QStringList UdevDevice::alternateDeviceSymlinks() const
{
    Q_D(const UdevDevice);
    return listFromEntries(udev_device_get_devlinks_list_entry(d->device));
}

QString UdevDevice::sysfsPath() const
{
    Q_D(const UdevDevice);
    return QString::fromUtf8(udev_device_get_syspath(d->device));
}

int UdevDevice::sysfsNumber() const
{
    Q_D(const UdevDevice);
    return QByteArray(udev_device_get_sysnum(d->device)).toInt();
}

QStringList UdevDevice::deviceProperties() const
{
    Q_D(const UdevDevice);
    return listFromEntries(udev_device_get_properties_list_entry(d->device));
}

QStringList UdevDevice::sysfsProperties() const
{
    Q_D(const UdevDevice);
    return listFromEntries(udev_device_get_sysattr_list_entry(d->device));
}

UdevDevice *UdevDevice::parent() const
{
    Q_D(const UdevDevice);

    udev_device *p = udev_device_get_parent(d->device);
    if (p)
        return new UdevDevice(p);
    return Q_NULLPTR;
}

QDebug operator<<(QDebug dbg, const UdevDevice &device)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "UdevDevice(" << device.deviceNode() << ')';
    return dbg;
}

} // namespace Platform

} // namespace GreenIsland
