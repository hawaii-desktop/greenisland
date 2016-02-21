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

#include "logging.h"
#include "udev/udev.h"
#include "udev/udev_p.h"
#include "udev/udevenumerate.h"

namespace GreenIsland {

namespace Platform {

/*
 * UdevEnumeratePrivate
 */

class UdevEnumeratePrivate
{
public:
    UdevEnumeratePrivate(UdevDevice::DeviceTypes t, Udev *u)
        : types(t)
        , udev(u)
    {
        enumerate = udev_enumerate_new(UdevPrivate::get(u)->udev);
        if (!enumerate) {
            qCWarning(lcUdev, "Unable to enumerate connected devices");
            return;
        }

        if (types.testFlag(UdevDevice::InputDevice_Mask))
            udev_enumerate_add_match_subsystem(enumerate, "input");

        if (types.testFlag(UdevDevice::VideoDevice_Mask)) {
            udev_enumerate_add_match_subsystem(enumerate, "drm");
            udev_enumerate_add_match_sysname(enumerate, "card[0-9]*");
        }

        if (types.testFlag(UdevDevice::KeyboardDevice)) {
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEYBOARD", "1");
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEY", "1");
        }

        if (types.testFlag(UdevDevice::MouseDevice))
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_MOUSE", "1");

        if (types.testFlag(UdevDevice::TouchpadDevice))
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_TOUCHPAD", "1");

        if (types.testFlag(UdevDevice::TouchscreenDevice))
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_TOUCHSCREEN", "1");

        if (types.testFlag(UdevDevice::TabletDevice))
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_TABLET", "1");

        if (types.testFlag(UdevDevice::JoystickDevice))
            udev_enumerate_add_match_property(enumerate, "ID_INPUT_JOYSTICK", "1");
    }

    ~UdevEnumeratePrivate()
    {
        if (enumerate)
            udev_enumerate_unref(enumerate);
    }

    UdevDevice::DeviceTypes types;
    Udev *udev;
    udev_enumerate *enumerate;
};

/*
 * UdevEnumerate
 */

UdevEnumerate::UdevEnumerate(UdevDevice::DeviceTypes types, Udev *udev)
    : d_ptr(new UdevEnumeratePrivate(types, udev))
{
}

UdevEnumerate::~UdevEnumerate()
{
    delete d_ptr;
}

QList<UdevDevice *> UdevEnumerate::scan() const
{
    Q_D(const UdevEnumerate);

    QList<UdevDevice *> list;

    if (!d->enumerate)
        return list;

    if (udev_enumerate_scan_devices(d->enumerate) != 0) {
        qCWarning(lcUdev, "Unable to enumerate connected devices");
        return list;
    }

    udev_device *drmDevice = Q_NULLPTR;
    udev_device *drmPrimaryDevice = Q_NULLPTR;

    udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(d->enumerate)) {
        const char *syspath = udev_list_entry_get_name(entry);
        udev_device *dev = udev_device_new_from_syspath(UdevPrivate::get(d->udev)->udev, syspath);
        if (!dev)
            continue;

        // Must be on the same seat
        QByteArray seat = QByteArray(udev_device_get_property_value(dev, "ID_SEAT"));
        if (seat.isEmpty())
            seat = QByteArrayLiteral("seat0");
        if (seat != qgetenv("XDG_SEAT")) {
            udev_device_unref(dev);
            continue;
        }

        QString node = QString::fromUtf8(udev_device_get_devnode(dev));

        if (d->types.testFlag(UdevDevice::InputDevice_Mask) && node.startsWith(QLatin1String("/dev/input/event")))
            list.append(new UdevDevice(dev));

        if (d->types.testFlag(UdevDevice::VideoDevice_Mask) && node.startsWith(QLatin1String("/dev/dri/card"))) {
            // We can have more than one DRM device on our seat, so the filter
            // might want us to pick up only the primary video device
            // In any case we'll be adding just one DRM device to the list
            if (d->types.testFlag(UdevDevice::PrimaryVideoDevice)) {
                udev_device *pci =
                        udev_device_get_parent_with_subsystem_devtype(dev, "pci", Q_NULLPTR);
                if (pci) {
                    if (qstrcmp(udev_device_get_sysattr_value(pci, "boot_vga"), "1") == 0)
                        drmPrimaryDevice = dev;
                }
            }
            if (!drmPrimaryDevice) {
                if (drmDevice)
                    udev_device_unref(drmDevice);
                drmDevice = dev;
            }
        }
    }

    // Add any DRM device previously enumerated
    if (drmPrimaryDevice)
        list.append(new UdevDevice(drmPrimaryDevice));
    else if (drmDevice)
        list.append(new UdevDevice(drmDevice));

    return list;
}

} // namespace Platform

} // namespace GreenIsland
