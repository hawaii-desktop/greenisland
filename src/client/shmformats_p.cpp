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

#include "shmformats_p.h"

namespace GreenIsland {

namespace Client {

static wl_shm_format formats[] = {
    wl_shm_format(INT_MIN),    // Format_Invalid
    wl_shm_format(INT_MIN),    // Format_Mono,
    wl_shm_format(INT_MIN),    // Format_MonoLSB,
    wl_shm_format(INT_MIN),    // Format_Indexed8,
    WL_SHM_FORMAT_XRGB8888,    // Format_RGB32,
    WL_SHM_FORMAT_ARGB8888,    // Format_ARGB32,
    WL_SHM_FORMAT_ARGB8888,    // Format_ARGB32_Premultiplied,
    WL_SHM_FORMAT_RGB565,      // Format_RGB16,
    wl_shm_format(INT_MIN),    // Format_ARGB8565_Premultiplied,
    wl_shm_format(INT_MIN),    // Format_RGB666,
    wl_shm_format(INT_MIN),    // Format_ARGB6666_Premultiplied,
    WL_SHM_FORMAT_XRGB1555,    // Format_RGB555,
    wl_shm_format(INT_MIN),    // Format_ARGB8555_Premultiplied,
    WL_SHM_FORMAT_RGB888,      // Format_RGB888,
    WL_SHM_FORMAT_XRGB4444,    // Format_RGB444,
    WL_SHM_FORMAT_ARGB4444,    // Format_ARGB4444_Premultiplied,
    WL_SHM_FORMAT_XBGR8888,    // Format_RGBX8888,
    WL_SHM_FORMAT_ABGR8888,    // Format_RGBA8888,
    WL_SHM_FORMAT_ABGR8888,    // Format_RGBA8888_Premultiplied,
    WL_SHM_FORMAT_XBGR2101010, // Format_BGR30,
    WL_SHM_FORMAT_ARGB2101010, // Format_A2BGR30_Premultiplied,
    WL_SHM_FORMAT_XRGB2101010, // Format_RGB30,
    WL_SHM_FORMAT_ARGB2101010, // Format_A2RGB30_Premultiplied,
    WL_SHM_FORMAT_C8,          // Format_Alpha8,
    WL_SHM_FORMAT_C8           // Format_Grayscale8,
};

static size_t getSize()
{
    return sizeof(formats) / sizeof(*formats);
}

wl_shm_format ShmFormats::fromQt(QImage::Format format)
{
    if (getSize() <= size_t(format))
        return wl_shm_format(INT_MIN);
    return formats[format];
}

QImage::Format ShmFormats::fromWayland(wl_shm_format format)
{
    const size_t size = getSize();
    for (size_t i = 0; i < size; i++) {
        if (formats[i] == format)
            return QImage::Format(i);
    }

    return QImage::Format_Invalid;
}

} // namespace Client

} // namespace GreenIsland
