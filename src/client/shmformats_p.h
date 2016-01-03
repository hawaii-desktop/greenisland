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
