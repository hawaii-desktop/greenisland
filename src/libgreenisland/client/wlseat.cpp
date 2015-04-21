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

#include "wlpointer.h"
#include "wlseat.h"

namespace GreenIsland {

WlSeat::WlSeat(WlRegistry *registry, wl_compositor *compositor,
               quint32 name, quint32 version)
    : QtWayland::wl_seat(registry->registry(), name, version)
    , m_version(version)
    , m_compositor(compositor)
    , m_pointer(Q_NULLPTR)
{
}

WlSeat::~WlSeat()
{
    delete m_pointer;
}

void WlSeat::seat_capabilities(uint32_t capabilities)
{
    if (capabilities & capability_pointer && !m_pointer) {
        m_pointer = new WlPointer(this);
        m_pointer->init(get_pointer());
    } else if (!(capabilities & capability_pointer) && m_pointer) {
        delete m_pointer;
        m_pointer = Q_NULLPTR;
    }
}

}
