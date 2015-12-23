/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
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

#ifndef GREENISLAND_EGLFSXKB_H
#define GREENISLAND_EGLFSXKB_H

#include <Qt>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <xkbcommon/xkbcommon.h>

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT EglFSXkb
{
public:
    static int keysymToQtKey(xkb_keysym_t keysym);
    static int keysymToQtKey(xkb_keysym_t keysym, Qt::KeyboardModifiers &modifiers, const QString &text);
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSXKB_H