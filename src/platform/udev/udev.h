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

#ifndef GREENISLAND_UDEV_H
#define GREENISLAND_UDEV_H

#include <QtCore/QString>

#include <GreenIsland/platform/greenislandplatform_export.h>

namespace GreenIsland {

namespace Platform {

class UdevDevice;
class UdevPrivate;

class GREENISLANDPLATFORM_EXPORT Udev
{
    Q_DECLARE_PRIVATE(Udev)
public:
    Udev();
    ~Udev();

    bool isValid() const;

    UdevDevice *deviceFromFileName(const QString &fileName) const;
    UdevDevice *deviceFromSubSystemAndName(const QString &subSystem, const QString &name) const;
    UdevDevice *deviceFromSysfsPath(const QString &sysfsPath) const;

private:
    UdevPrivate *const d_ptr;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_UDEV_H
