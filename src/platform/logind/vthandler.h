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

#ifndef GREENISLAND_VTHANDLER_H
#define GREENISLAND_VTHANDLER_H

#include <QtCore/QObject>

#include <GreenIsland/platform/greenislandplatform_export.h>

namespace GreenIsland {

namespace Platform {

class VtHandlerPrivate;

class GREENISLANDPLATFORM_EXPORT VtHandler : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VtHandler)
public:
    VtHandler(QObject *parent = 0);

    bool isActive() const;

    void activate(quint32 nr);
    void suspend();

Q_SIGNALS:
    void created();
    void activeChanged(bool active);
    void interrupted();
    void aboutToSuspend();
    void resumed();
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_VTHANDLER_H
