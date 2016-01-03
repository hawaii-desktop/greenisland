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

#ifndef GREENISLAND_COMPOSITORSETTINGS_P_H
#define GREENISLAND_COMPOSITORSETTINGS_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Server/CompositorSettings>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace GreenIsland {

namespace Server {

class CompositorSettingsPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(CompositorSettings)
public:
    CompositorSettingsPrivate()
        : initialized(false)
        , compositor(Q_NULLPTR)
        , windowActionKey(Qt::MetaModifier)
        , repeatRate(40)
        , repeatDelay(400)
        , keymap(Q_NULLPTR)
    {
    }

    void _q_setupKeymap();

    bool initialized;
    QWaylandCompositor *compositor;
    Qt::KeyboardModifier windowActionKey;
    quint32 repeatRate;
    quint32 repeatDelay;
    Keymap *keymap;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_COMPOSITORSETTINGS_P_H
