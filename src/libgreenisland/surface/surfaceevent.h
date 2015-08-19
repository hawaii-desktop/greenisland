/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLAND_SURFACEEVENT_H
#define GREENISLAND_SURFACEEVENT_H

#include <QtCore/QEvent>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class Output;

class SurfaceEnterEventPrivate;

class GREENISLAND_EXPORT SurfaceEnterEvent : public QEvent
{
public:
    SurfaceEnterEvent(Output *output);
    ~SurfaceEnterEvent();

    Output *output() const;

    static const QEvent::Type WaylandSurfaceEnter;

private:
    SurfaceEnterEventPrivate *d;
};

class SurfaceLeaveEventPrivate;

class GREENISLAND_EXPORT SurfaceLeaveEvent : public QEvent
{
public:
    SurfaceLeaveEvent(Output *output);
    ~SurfaceLeaveEvent();

    Output *output() const;

    static const QEvent::Type WaylandSurfaceLeave;

private:
    SurfaceLeaveEventPrivate *d;
};

} // namespace GreenIsland

#endif // GREENISLAND_SURFACEEVENT_H

