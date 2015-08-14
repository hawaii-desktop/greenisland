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

#include "abstractoutput.h"
#include "surfaceevent.h"
#include "surfaceevent_p.h"

namespace GreenIsland {

const QEvent::Type SurfaceEnterEvent::WaylandSurfaceEnter = (QEvent::Type)QEvent::registerEventType();
const QEvent::Type SurfaceLeaveEvent::WaylandSurfaceLeave = (QEvent::Type)QEvent::registerEventType();

/*
 * SurfaceEnterEventPrivate
 */

SurfaceEnterEventPrivate::SurfaceEnterEventPrivate(AbstractOutput *_output)
    : output(_output)
{
}

/*
 * SurfaceEnterEvent
 */

SurfaceEnterEvent::SurfaceEnterEvent(AbstractOutput *output)
    : QEvent(WaylandSurfaceEnter)
    , d(new SurfaceEnterEventPrivate(output))
{
}

SurfaceEnterEvent::~SurfaceEnterEvent()
{
    delete d;
}

AbstractOutput *SurfaceEnterEvent::output() const
{
    return d->output;
}

/*
 * SurfaceLeaveEventPrivate
 */

SurfaceLeaveEventPrivate::SurfaceLeaveEventPrivate(AbstractOutput *_output)
    : output(_output)
{
}

/*
 * SurfaceLeaveEvent
 */

SurfaceLeaveEvent::SurfaceLeaveEvent(AbstractOutput *output)
    : QEvent(WaylandSurfaceLeave)
    , d(new SurfaceLeaveEventPrivate(output))
{
}

SurfaceLeaveEvent::~SurfaceLeaveEvent()
{
    delete d;
}

AbstractOutput *SurfaceLeaveEvent::output() const
{
    return d->output;
}

} // namespace GreenIsland

