/****************************************************************************
 * This file is part of libGreenIsland.
 *
 * Copyright (c) Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
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

#ifndef VGREENISLANDGLOBAL_H
#define VGREENISLANDGLOBAL_H

#include <qglobal.h>

#if defined(GREENISLAND_LIB)
#  define GREENISLAND_EXPORT Q_DECL_EXPORT
#else
#  define GREENISLAND_EXPORT Q_DECL_IMPORT
#endif

#endif // VGREENISLANDGLOBAL_H
