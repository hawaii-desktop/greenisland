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

#ifndef GREENISLANDCLIENT_REGION_H
#define GREENISLANDCLIENT_REGION_H

#include <QtCore/QObject>
#include <QtGui/QRegion>

#include <GreenIsland/client/greenislandclient_export.h>

namespace GreenIsland {

namespace Client {

class RegionPrivate;

class GREENISLANDCLIENT_EXPORT Region : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Region)
public:
    Region(const QRegion &region, QObject *parent = Q_NULLPTR);

    bool isInitialized() const;

    void add(const QRegion &region);
    void add(const QRect &rect);

    void subtract(const QRegion &region);
    void subtract(const QRect &rect);

    static QByteArray interfaceName();
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_REGION_H
