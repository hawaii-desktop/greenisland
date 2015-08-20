/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Giulio Camuffo
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in
 *         the documentation and/or other materials provided with the
 *         distribution.
 *       * Neither the name of The Qt Company Ltd nor the names of its
 *         contributors may be used to endorse or promote products derived
 *         from this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include "surfaceinterface.h"
#include "surface.h"
#include "surface_p.h"

#include <wayland-server.h>

namespace GreenIsland {

class SurfaceInterface::Private
{
public:
    Surface *surface;
};

SurfaceInterface::SurfaceInterface(Surface *surface)
                   : d(new Private)
{
    d->surface = surface;
    surface->addInterface(this);
}

SurfaceInterface::~SurfaceInterface()
{
    d->surface->removeInterface(this);
    delete d;
}

Surface *SurfaceInterface::surface() const
{
    return d->surface;
}

void SurfaceInterface::setSurfaceType(Surface::WindowType type)
{
    surface()->d_func()->setType(type);
}

void SurfaceInterface::setSurfaceClassName(const QString &name)
{
    surface()->d_func()->setClassName(name);
}

void SurfaceInterface::setSurfaceTitle(const QString &title)
{
    surface()->d_func()->setTitle(title);
}



class SurfaceOperation::Private
{
public:
    int type;
};

SurfaceOperation::SurfaceOperation(int t)
                 : d(new Private)
{
    d->type = t;
}

SurfaceOperation::~SurfaceOperation()
{
    delete d;
}

int SurfaceOperation::type() const
{
    return d->type;
}



SurfaceSetVisibilityOperation::SurfaceSetVisibilityOperation(QWindow::Visibility visibility)
                              : SurfaceOperation(SurfaceOperation::SetVisibility)
                              , m_visibility(visibility)
{
}

QWindow::Visibility SurfaceSetVisibilityOperation::visibility() const
{
    return m_visibility;
}

SurfaceResizeOperation::SurfaceResizeOperation(const QSize &size)
                       : SurfaceOperation(SurfaceOperation::Resize)
                       , m_size(size)
{
}

QSize SurfaceResizeOperation::size() const
{
    return m_size;
}

SurfacePingOperation::SurfacePingOperation(uint32_t serial)
                     : SurfaceOperation(SurfaceOperation::Ping)
                     , m_serial(serial)
{
}

uint32_t SurfacePingOperation::serial() const
{
    return m_serial;
}

} // namespace GreenIsland
