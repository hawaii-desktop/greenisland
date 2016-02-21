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

#ifndef GREENISLAND_NATIVESCREENBACKEND_H
#define GREENISLAND_NATIVESCREENBACKEND_H

#include <GreenIsland/Server/ScreenBackend>

class QScreen;

namespace GreenIsland {

namespace Server {

class GREENISLANDSERVER_EXPORT NativeScreenBackend : public ScreenBackend
{
    Q_OBJECT
public:
    NativeScreenBackend(QObject *parent = Q_NULLPTR);

    void acquireConfiguration() Q_DECL_OVERRIDE;

private:
    bool m_initialized;

private Q_SLOTS:
    void handleScreenAdded(QScreen *qscreen);
    void handleScreenRemoved(QScreen *qscreen);
    void handleScreenChanged(QScreen *qscreen, Screen *screen);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_NATIVESCREENBACKEND_H
