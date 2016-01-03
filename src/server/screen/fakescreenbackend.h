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

#ifndef GREENISLAND_FAKESCREENBACKEND_H
#define GREENISLAND_FAKESCREENBACKEND_H

#include <GreenIsland/Server/ScreenBackend>

namespace GreenIsland {

namespace Server {

class GREENISLANDSERVER_EXPORT FakeScreenBackend : public ScreenBackend
{
    Q_OBJECT
public:
    FakeScreenBackend(QObject *parent = Q_NULLPTR);

    void setConfiguration(const QString &fileName);

    void acquireConfiguration() Q_DECL_OVERRIDE;

private:
    QString m_fileName;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_FAKESCREENBACKEND_H
