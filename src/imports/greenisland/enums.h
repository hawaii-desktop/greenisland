/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>

class LauncherAlignment : public QObject
{
    Q_OBJECT
    Q_ENUMS(Alignment)
public:
    enum Alignment {
        Left,
        Right,
        Bottom
    };
};

class UserStatus : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)
public:
    enum Status {
        Offline,
        Available,
        Busy,
        Invisible,
        Away,
        Idle,
        Pending,
        Locked
    };
};

#endif // ENUMS_H
