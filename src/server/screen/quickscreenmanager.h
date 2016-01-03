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

#ifndef GREENISLAND_QUICKSCREENMANAGER_H
#define GREENISLAND_QUICKSCREENMANAGER_H

#include <QtQml/QQmlParserStatus>

#include <GreenIsland/Server/ScreenManager>

namespace GreenIsland {

namespace Server {

class GREENISLANDSERVER_EXPORT QuickScreenManager : public ScreenManager, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
public:
    QuickScreenManager(QObject *parent = Q_NULLPTR);

    void create() Q_DECL_OVERRIDE;

protected:
    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_QUICKSCREENMANAGER_H
