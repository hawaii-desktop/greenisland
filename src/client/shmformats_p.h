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

#ifndef GREENISLANDCLIENT_SHMFORMATS_P_H
#define GREENISLANDCLIENT_SHMFORMATS_P_H

#include <QtGui/QImage>

#include <GreenIsland/client/greenislandclient_export.h>

#include <wayland-client.h>

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT ShmFormats
{
public:
    static wl_shm_format fromQt(QImage::Format format);
    static QImage::Format fromWayland(wl_shm_format format);
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SHMFORMATS_P_H
