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
