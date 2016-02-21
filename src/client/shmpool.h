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

#ifndef GREENISLANDCLIENT_SHMPOOL_H
#define GREENISLANDCLIENT_SHMPOOL_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/Shm>

namespace GreenIsland {

namespace Client {

class ShmPoolPrivate;

class GREENISLANDCLIENT_EXPORT ShmPool : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ShmPool)
public:
    Shm *shm() const;

    void *address() const;

    BufferPtr createBuffer(const QImage &image);
    BufferPtr createBuffer(const QSize &size, quint32 stride,
                           const void *source = Q_NULLPTR,
                           Shm::Format format = Shm::Format_ARGB32);

    BufferPtr findBuffer(const QSize &size, quint32 stride,
                         Shm::Format format = Shm::Format_ARGB32);

    static QByteArray interfaceName();

Q_SIGNALS:
    void resized();

private:
    ShmPool(Shm *shm);

    friend class Shm;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SHMPOOL_H
