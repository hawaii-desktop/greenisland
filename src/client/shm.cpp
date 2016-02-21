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

#include "shm.h"
#include "shm_p.h"
#include "shmpool.h"
#include "shmpool_p.h"

namespace GreenIsland {

namespace Client {

/*
 * ShmPrivate
 */

ShmPrivate::ShmPrivate()
    : QtWayland::wl_shm()
{
}

void ShmPrivate::shm_format(uint32_t format)
{
    Q_Q(Shm);

    Shm::Format f = static_cast<Shm::Format>(format);
    formats.append(f);
    Q_EMIT q->formatAdded(f);
}

/*
 * Shm
 */

Shm::Shm(QObject *parent)
    : QObject(*new ShmPrivate(), parent)
{
}

QList<Shm::Format> Shm::formats() const
{
    Q_D(const Shm);
    return d->formats.toList();
}

ShmPool *Shm::createPool(size_t size)
{
    Q_D(Shm);
    ShmPool *pool = new ShmPool(this);
    ShmPoolPrivate *dPool = ShmPoolPrivate::get(pool);
    if (!dPool->createPool(this, size)) {
        delete pool;
        return Q_NULLPTR;
    }
    return pool;
}

QString Shm::formatToString(Format format)
{
    switch (format) {
    case Format_ARGB32:
        return QLatin1String("ARGB32");
    case Format_RGB32:
        return QLatin1String("RGB32");
    case Format_C8:
        return QLatin1String("C8");
    case Format_RGB332:
        return QLatin1String("RGB332");
    case Format_BGR233:
        return QLatin1String("BGR233");
    case Format_XRGB4444:
        return QLatin1String("XRGB4444");
    case Format_XBGR4444:
        return QLatin1String("XBGR4444");
    case Format_RGBX4444:
        return QLatin1String("RGBX4444");
    case Format_BGRX4444:
        return QLatin1String("BGRX4444");
    case Format_ARGB4444:
        return QLatin1String("ARGB4444");
    case Format_ABGR4444:
        return QLatin1String("ABGR4444");
    case Format_RGBA4444:
        return QLatin1String("RGBA4444");
    case Format_BGRA4444:
        return QLatin1String("BGRA4444");
    case Format_XRGB1555:
        return QLatin1String("XRGB15555");
    case Format_XBGR1555:
        return QLatin1String("XBGR1555");
    case Format_RGBX5551:
        return QLatin1String("RGBX5551");
    case Format_BGRX5551:
        return QLatin1String("BGRX5551");
    case Format_ARGB1555:
        return QLatin1String("ARGB1555");
    case Format_ABGR1555:
        return QLatin1String("ABGR1555");
    case Format_RGBA5551:
        return QLatin1String("RGBA5551");
    case Format_BGRA5551:
        return QLatin1String("BGRA5551");
    case Format_RGB565:
        return QLatin1String("RGB565");
    case Format_BGR565:
        return QLatin1String("BGR565");
    case Format_RGB888:
        return QLatin1String("RGB888");
    case Format_BGR888:
        return QLatin1String("BGR888");
    case Format_XBGR8888:
        return QLatin1String("XBGR8888");
    case Format_RGBX8888:
        return QLatin1String("RGBX8888");
    case Format_BGRX8888:
        return QLatin1String("BGRX8888");
    case Format_ABGR8888:
        return QLatin1String("ABGR8888");
    case Format_RGBA8888:
        return QLatin1String("RGBA8888");
    case Format_BGRA8888:
        return QLatin1String("BGRA8888");
    case Format_XRGB2101010:
        return QLatin1String("XRGB2101010");
    case Format_XBGR2101010:
        return QLatin1String("XGBGR2101010");
    case Format_RGBX1010102:
        return QLatin1String("RGBX1010102");
    case Format_BGRX1010102:
        return QLatin1String("BGRX1010102");
    case Format_ARGB2101010:
        return QLatin1String("ARGB2101010");
    case Format_ABGR2101010:
        return QLatin1String("ABGR2101010");
    case Format_RGBA1010102:
        return QLatin1String("RGBA1010102");
    case Format_BGRA1010102:
        return QLatin1String("BGRA1010102");
    case Format_YUYV:
        return QLatin1String("YUYV");
    case Format_YVYU:
        return QLatin1String("YVYU");
    case Format_UYVY:
        return QLatin1String("UYVY");
    case Format_VYUY:
        return QLatin1String("VYUY");
    case Format_AYUV:
        return QLatin1String("AYUV");
    case Format_NV12:
        return QLatin1String("NV12");
    case Format_NV21:
        return QLatin1String("NV21");
    case Format_NV16:
        return QLatin1String("NV16");
    case Format_NV61:
        return QLatin1String("NV61");
    case Format_YUV410:
        return QLatin1String("YUV410");
    case Format_YVU410:
        return QLatin1String("YVU410");
    case Format_YUV411:
        return QLatin1String("YUV411");
    case Format_YVU411:
        return QLatin1String("YVU411");
    case Format_YUV420:
        return QLatin1String("YUV420");
    case Format_YVU420:
        return QLatin1String("YVU420");
    case Format_YUV422:
        return QLatin1String("YUV422");
    case Format_YVU422:
        return QLatin1String("YVU422");
    case Format_YUV444:
        return QLatin1String("YUV444");
    case Format_YVU444:
        return QLatin1String("YVU444");
    }

    Q_UNREACHABLE();
}

QByteArray Shm::interfaceName()
{
    return QByteArrayLiteral("wl_shm");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_shm.cpp"
