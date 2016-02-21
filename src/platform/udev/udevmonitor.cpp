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

#include <QtCore/QSocketNotifier>
#include <QtCore/private/qobject_p.h>

#include "logging.h"
#include "udev/udev.h"
#include "udev/udev_p.h"
#include "udev/udevdevice.h"
#include "udev/udevmonitor.h"

namespace GreenIsland {

namespace Platform {

/*
 * UdevMonitorPrivate
 */

class UdevMonitorPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UdevMonitor)
public:
    UdevMonitorPrivate(Udev *u)
        : udev(u)
        , monitor(Q_NULLPTR)
    {
        monitor = udev_monitor_new_from_netlink(UdevPrivate::get(u)->udev, "udev");
        if (!monitor) {
            qCWarning(lcUdev, "Unable to create an udev monitor: no devices can be detected");
            return;
        }

        udev_monitor_enable_receiving(monitor);
    }

    ~UdevMonitorPrivate()
    {
        if (monitor)
            udev_monitor_unref(monitor);
    }

    void _q_udevEventHandler()
    {
        Q_Q(UdevMonitor);

        udev_device *dev = udev_monitor_receive_device(monitor);
        if (!dev)
            return;

        const char *action = udev_device_get_action(dev);
        if (!action) {
            udev_device_unref(dev);
            return;
        }

        UdevDevice *device = new UdevDevice(dev);

        if (strcmp(action, "add") == 0)
            Q_EMIT q->deviceAdded(device);
        else if (strcmp(action, "remove") == 0)
            Q_EMIT q->deviceRemoved(device);
        else if (strcmp(action, "change") == 0)
            Q_EMIT q->deviceChanged(device);
        else if (strcmp(action, "online") == 0)
            Q_EMIT q->deviceOnlined(device);
        else if (strcmp(action, "offline") == 0)
            Q_EMIT q->deviceOfflined(device);
    }

    Udev *udev;
    struct udev_monitor *monitor;
};

/*
 * UdevMonitor
 */

UdevMonitor::UdevMonitor(Udev *udev, QObject *parent)
    : QObject(*new UdevMonitorPrivate(udev), parent)
{
    Q_D(UdevMonitor);
    int fd = udev_monitor_get_fd(d->monitor);
    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(_q_udevEventHandler()));
}

bool UdevMonitor::isValid() const
{
    Q_D(const UdevMonitor);
    return d->monitor;
}

void UdevMonitor::filterSubSystemDevType(const QString &subSystem, const QString &devType)
{
    Q_D(UdevMonitor);

    if (!isValid())
        return;

    udev_monitor_filter_add_match_subsystem_devtype(d->monitor,
                                                    qPrintable(subSystem),
                                                    qPrintable(devType));
}

void UdevMonitor::filterTag(const QString &tag)
{
    Q_D(UdevMonitor);

    if (!isValid())
        return;

    udev_monitor_filter_add_match_tag(d->monitor, qPrintable(tag));
}

} // namespace Platform

} // namespace GreenIsland
