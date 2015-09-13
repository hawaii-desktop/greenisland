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

#ifndef WLSHMPOOL_H
#define WLSHMPOOL_H

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(WLSHMPOOL)

struct wl_shm;

namespace GreenIsland {

class WlRegistry;
class WlShmPoolPrivate;

class WlShmPool : public QObject
{
    Q_OBJECT
public:
    ~WlShmPool();

    bool isValid() const;

    wl_shm *shm() const;

Q_SIGNALS:
    void resized();

private:
    Q_DECLARE_PRIVATE(WlShmPool)
    WlShmPoolPrivate *const d_ptr;

    WlShmPool(wl_shm *shm, QObject *parent = 0);

    friend class WlRegistry;
};

}

#endif // WLSHMPOOL_H
