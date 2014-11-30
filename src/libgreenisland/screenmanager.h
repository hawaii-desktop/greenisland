/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <QtCore/QObject>

namespace KScreen {
class Output;
}

namespace GreenIsland {

class Compositor;
class ScreenManagerPrivate;

class ScreenManager : public QObject
{
    Q_OBJECT
public:
    explicit ScreenManager(Compositor *compositor);
    ~ScreenManager();

private:
    Q_DECLARE_PRIVATE(ScreenManager)
    ScreenManagerPrivate *const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_configurationChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_outputAdded(const KScreen::OutputPtr &output))
    Q_PRIVATE_SLOT(d_func(), void _q_outputRemoved(int id))
};

}

#endif // SCREENMANAGER_H
