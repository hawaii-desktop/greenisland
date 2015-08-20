/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef GREENISLAND_SURFACEROLEHANDLER_H
#define GREENISLAND_SURFACEROLEHANDLER_H

#include <GreenIsland/Surface>

namespace GreenIsland {

class SurfaceRoleHandlerPrivate;

class GREENISLAND_EXPORT SurfaceRoleHandler
{
    Q_DECLARE_PRIVATE(SurfaceRoleHandler)
public:
    SurfaceRoleHandler();
    virtual ~SurfaceRoleHandler();

    virtual void configure(int x, int y) = 0;

    static QString name();

private:
    SurfaceRoleHandlerPrivate *const d_ptr;

    friend class Surface;
};

} // namespace GreenIsland

#endif // GREENISLAND_SURFACEROLEHANDLER_H
