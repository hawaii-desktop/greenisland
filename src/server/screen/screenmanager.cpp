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

#include "screenmanager.h"
#include "screenmanager_p.h"

namespace GreenIsland {

namespace Server {

ScreenManager::ScreenManager(QObject *parent)
    : QObject(*new ScreenManagerPrivate(), parent)
{
    Q_D(ScreenManager);

    connect(d->backend, &ScreenBackend::screenAdded,
            this, &ScreenManager::screenAdded);
    connect(d->backend, &ScreenBackend::screenRemoved,
            this, &ScreenManager::screenRemoved);
    connect(d->backend, &ScreenBackend::primaryScreenChanged, this,
            [this, d](Screen *screen) {
        d->primaryScreen = screen;
        Q_EMIT primaryScreenChanged(screen);
    });
}

Screen *ScreenManager::primaryScreen() const
{
    Q_D(const ScreenManager);
    return d->primaryScreen;
}

int ScreenManager::indexOf(Screen *screen) const
{
    Q_D(const ScreenManager);
    return d->backend->screens().indexOf(screen);
}

void ScreenManager::create()
{
    Q_D(ScreenManager);
    d->backend->acquireConfiguration();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_screenmanager.cpp"
