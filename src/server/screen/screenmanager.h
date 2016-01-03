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

#ifndef GREENISLAND_SCREENMANAGER_H
#define GREENISLAND_SCREENMANAGER_H

#include <GreenIsland/Server/Screen>

namespace GreenIsland {

namespace Server {

class ScreenManagerPrivate;

class GREENISLANDSERVER_EXPORT ScreenManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ScreenManager)
    Q_PROPERTY(Screen *primaryScreen READ primaryScreen NOTIFY primaryScreenChanged)
public:
    ScreenManager(QObject *parent = Q_NULLPTR);

    Screen *primaryScreen() const;

    Q_INVOKABLE int indexOf(Screen *screen) const;

    virtual void create();

Q_SIGNALS:
    void screenAdded(Screen *screen);
    void screenRemoved(Screen *screen);
    void primaryScreenChanged(Screen *screen);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENMANAGER_H
