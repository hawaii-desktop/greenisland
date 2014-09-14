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

#ifndef GLOBALSHORTCUTSMANAGER_H
#define GLOBALSHORTCUTSMANAGER_H

#include <QtCore/QObject>
#include <QtCompositor/private/qwlkeyboard_p.h>

namespace QtWayland {
class Surface;
}

class GlobalShortcutsManager : public QObject, public QtWayland::KeyboardGrabber
{
    Q_OBJECT
public:
    explicit GlobalShortcutsManager(QObject *parent = 0);

    void focused(QtWayland::Surface *surface) Q_DECL_OVERRIDE;
    void key(uint32_t serial, uint32_t time, uint32_t key,
             uint32_t state) Q_DECL_OVERRIDE;
    void modifiers(uint32_t serial, uint32_t mods_depressed,
                   uint32_t mods_latched, uint32_t mods_locked,
                   uint32_t group) Q_DECL_OVERRIDE;
};

#endif // GLOBALSHORTCUTSMANAGER_H
