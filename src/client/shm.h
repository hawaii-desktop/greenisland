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

#ifndef GREENISLANDCLIENT_SHM_H
#define GREENISLANDCLIENT_SHM_H

#include <QtCore/QObject>

#include <GreenIsland/client/greenislandclient_export.h>

namespace GreenIsland {

namespace Client {

class Registry;
class ShmPool;
class ShmPrivate;

class GREENISLANDCLIENT_EXPORT Shm : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Shm)
public:
    enum Format {
        Format_ARGB32 = 0,
        Format_RGB32 = 1,
        Format_C8 = 0x20203843,
        Format_RGB332 = 0x38424752,
        Format_BGR233 = 0x38524742,
        Format_XRGB4444 = 0x32315258,
        Format_XBGR4444 = 0x32314258,
        Format_RGBX4444 = 0x32315852,
        Format_BGRX4444 = 0x32315842,
        Format_ARGB4444 = 0x32315241,
        Format_ABGR4444 = 0x32314241,
        Format_RGBA4444 = 0x32314152,
        Format_BGRA4444 = 0x32314142,
        Format_XRGB1555 = 0x35315258,
        Format_XBGR1555 = 0x35314258,
        Format_RGBX5551 = 0x35315852,
        Format_BGRX5551 = 0x35315842,
        Format_ARGB1555 = 0x35315241,
        Format_ABGR1555 = 0x35314241,
        Format_RGBA5551 = 0x35314152,
        Format_BGRA5551 = 0x35314142,
        Format_RGB565 = 0x36314752,
        Format_BGR565 = 0x36314742,
        Format_RGB888 = 0x34324752,
        Format_BGR888 = 0x34324742,
        Format_XBGR8888 = 0x34324258,
        Format_RGBX8888 = 0x34325852,
        Format_BGRX8888 = 0x34325842,
        Format_ABGR8888 = 0x34324241,
        Format_RGBA8888 = 0x34324152,
        Format_BGRA8888 = 0x34324142,
        Format_XRGB2101010 = 0x30335258,
        Format_XBGR2101010 = 0x30334258,
        Format_RGBX1010102 = 0x30335852,
        Format_BGRX1010102 = 0x30335842,
        Format_ARGB2101010 = 0x30335241,
        Format_ABGR2101010 = 0x30334241,
        Format_RGBA1010102 = 0x30334152,
        Format_BGRA1010102 = 0x30334142,
        Format_YUYV = 0x56595559,
        Format_YVYU = 0x55595659,
        Format_UYVY = 0x59565955,
        Format_VYUY = 0x59555956,
        Format_AYUV = 0x56555941,
        Format_NV12 = 0x3231564e,
        Format_NV21 = 0x3132564e,
        Format_NV16 = 0x3631564e,
        Format_NV61 = 0x3136564e,
        Format_YUV410 = 0x39565559,
        Format_YVU410 = 0x39555659,
        Format_YUV411 = 0x31315559,
        Format_YVU411 = 0x31315659,
        Format_YUV420 = 0x32315559,
        Format_YVU420 = 0x32315659,
        Format_YUV422 = 0x36315559,
        Format_YVU422 = 0x36315659,
        Format_YUV444 = 0x34325559,
        Format_YVU444 = 0x34325659
    };
    Q_ENUM(Format)

    QList<Format> formats() const;

    ShmPool *createPool(size_t size);

    static QString formatToString(Format format);
    static QByteArray interfaceName();

Q_SIGNALS:
    void formatAdded(Shm::Format format);

private:
    explicit Shm(QObject *parent = Q_NULLPTR);

    friend class Registry;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SHM_H
