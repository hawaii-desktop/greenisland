/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef FAKESCREENBACKEND_H
#define FAKESCREENBACKEND_H

#include "screenbackend.h"

Q_DECLARE_LOGGING_CATEGORY(FAKE_BACKEND)

namespace GreenIsland {

class ScreenConfiguration;
class ScreenOutput;

class FakeScreenBackend : public ScreenBackend
{
    Q_OBJECT
public:
    FakeScreenBackend(Compositor *compositor, QObject *parent = 0);
    ~FakeScreenBackend();

    void loadConfiguration(const QString &fileName);

public Q_SLOTS:
    void acquireConfiguration();

private:
    ScreenConfiguration *m_config;
    QMap<ScreenOutput *, Output *> m_outputMap;

private Q_SLOTS:
    void screenAdded(ScreenOutput *so);
    void screenRemoved(ScreenOutput *so);
};

}

#endif // FAKESCREENBACKEND_H
