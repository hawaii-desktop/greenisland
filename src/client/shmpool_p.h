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

#ifndef GREENISLANDCLIENT_SHMPOOL_P_H
#define GREENISLANDCLIENT_SHMPOOL_P_H

#include <QtCore/QLoggingCategory>
#include <QtCore/QTemporaryFile>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/ShmPool>
#include <GreenIsland/client/private/qwayland-wayland.h>

#include <unistd.h>
#include <sys/mman.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

Q_DECLARE_LOGGING_CATEGORY(WLSHMPOOL)

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT ShmPoolPrivate
        : public QObjectPrivate
        , public QtWayland::wl_shm_pool
{
    Q_DECLARE_PUBLIC(ShmPool)
public:
    ShmPoolPrivate();
    ~ShmPoolPrivate();

    bool createPool(Shm *shm, size_t createSize);
    bool resizePool(size_t newSize);

    QVector<BufferSharedPtr>::iterator reuseBuffer(const QSize &s, qint32 stride, Shm::Format format);

    static ShmPoolPrivate *get(ShmPool *pool) { return pool->d_func(); }

    Shm *shm;
    QScopedPointer<QTemporaryFile> file;
    uchar *data;
    qint32 size;
    qint32 offset;
    QVector<BufferSharedPtr> buffers;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SHMPOOL_P_H
