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

#ifndef GREENISLAND_UDEVMONITOR_H
#define GREENISLAND_UDEVMONITOR_H

#include <QtCore/QObject>

#include <GreenIsland/platform/greenislandplatform_export.h>

namespace GreenIsland {

namespace Platform {

class Udev;
class UdevDevice;
class UdevMonitorPrivate;

class GREENISLANDPLATFORM_EXPORT UdevMonitor : public QObject
{
    Q_DECLARE_PRIVATE(UdevMonitor)
public:
    UdevMonitor(Udev *udev, QObject *parent = 0);

    bool isValid() const;

    void filterSubSystemDevType(const QString &subSystem, const QString &devType);
    void filterTag(const QString &tag);

Q_SIGNALS:
    void deviceAdded(UdevDevice *device);
    void deviceRemoved(UdevDevice *device);
    void deviceChanged(UdevDevice *device);
    void deviceOnlined(UdevDevice *device);
    void deviceOfflined(UdevDevice *device);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_udevEventHandler())
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_UDEVMONITOR_H
