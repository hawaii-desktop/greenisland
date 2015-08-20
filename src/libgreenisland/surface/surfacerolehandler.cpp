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

#include "surfacerolehandler.h"
#include "surfacerolehandler_p.h"
#include "surface_p.h"

namespace GreenIsland {

/*
 * SurfaceRoleHandlerPrivate
 */

SurfaceRoleHandlerPrivate::SurfaceRoleHandlerPrivate()
    : surface(Q_NULLPTR)
{
}

/*
 * AbstractSurfaceRole
 */

SurfaceRoleHandler::SurfaceRoleHandler()
    : d_ptr(new SurfaceRoleHandlerPrivate)
{
}

SurfaceRoleHandler::~SurfaceRoleHandler()
{
    Q_D(SurfaceRoleHandler);

    if (d->surface) {
        d->surface->d_func()->roleHandler = Q_NULLPTR;
        d->surface->d_func()->roleName = QString();
        d->surface = Q_NULLPTR;
    }

    delete d_ptr;
}

QString SurfaceRoleHandler::name()
{
    return QString();
}

} // namespace GreenIsland
