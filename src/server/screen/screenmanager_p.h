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

#ifndef GREENISLAND_SCREENMANAGER_P_H
#define GREENISLAND_SCREENMANAGER_P_H

#include <QtCore/QCoreApplication>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Server/ScreenManager>

#include "screen/fakescreenbackend.h"
#include "screen/nativescreenbackend.h"

namespace GreenIsland {

namespace Server {

class GREENISLANDSERVER_EXPORT ScreenManagerPrivate : public QObjectPrivate
{
public:
    ScreenManagerPrivate()
        : QObjectPrivate()
        , primaryScreen(Q_NULLPTR)
    {
        // Determine the backend to use from the application
        const QString backendName = QCoreApplication::instance()->property("__greenisland_screen_backend").toString();
        const QString fileName = QCoreApplication::instance()->property("__greenisland_screen_configuration").toString();
        if (backendName == QLatin1String("fake") && !fileName.isEmpty()) {
            backend = new FakeScreenBackend();
            static_cast<FakeScreenBackend *>(backend)->setConfiguration(fileName);
        } else {
            backend = new NativeScreenBackend();
        }
    }

    ~ScreenManagerPrivate()
    {
        delete backend;
    }

    ScreenBackend *backend;
    Screen *primaryScreen;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENMANAGER_P_H

