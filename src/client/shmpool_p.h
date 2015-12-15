/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef GREENISLANDCLIENT_SHMPOOL_P_H
#define GREENISLANDCLIENT_SHMPOOL_P_H

#include <QtCore/QLoggingCategory>
#include <QtCore/QTemporaryFile>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/client/greenislandclient_export.h>

#include <unistd.h>
#include <sys/mman.h>

#include <wayland-client-protocol.h>

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

class GREENISLANDCLIENT_EXPORT ShmPoolPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(ShmPool)
public:
    ShmPoolPrivate();
    ~ShmPoolPrivate();

    bool create();
    bool resize(size_t newSize);
    void release();

    wl_shm *shm;
    wl_shm_pool *pool;
    QScopedPointer<QTemporaryFile> file;
    uchar *data;
    size_t size;
    bool valid;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SHMPOOL_P_H
