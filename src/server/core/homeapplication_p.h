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

#ifndef GREENISLAND_HOMEAPPLICATION_P_H
#define GREENISLAND_HOMEAPPLICATION_P_H

#include <QtCore/QFileInfo>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Server/HomeApplication>

namespace GreenIsland {

namespace Server {

static int convertPermission(const QFileInfo &fileInfo)
{
    int p = 0;

    if (fileInfo.permission(QFile::ReadUser))
        p += 400;
    if (fileInfo.permission(QFile::WriteUser))
        p += 200;
    if (fileInfo.permission(QFile::ExeUser))
        p += 100;
    if (fileInfo.permission(QFile::ReadGroup))
        p += 40;
    if (fileInfo.permission(QFile::WriteGroup))
        p += 20;
    if (fileInfo.permission(QFile::ExeGroup))
        p += 10;
    if (fileInfo.permission(QFile::ReadOther))
        p += 4;
    if (fileInfo.permission(QFile::WriteOther))
        p += 2;
    if (fileInfo.permission(QFile::ExeOther))
        p += 1;

    return p;
}

class GREENISLANDSERVER_EXPORT HomeApplicationPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(HomeApplication)
public:
    HomeApplicationPrivate()
        : initialized(false)
        , running(false)
        , notify(true)
        , engine(Q_NULLPTR)
    {
    }

    void verifyXdgRuntimeDir();

    bool initialized;
    bool running;
    bool notify;
    QString screenConfiguration;
    QQmlApplicationEngine *engine;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_HOMEAPPLICATION_P_H

