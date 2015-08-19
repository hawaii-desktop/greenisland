/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <wayland-server.h>

#include "surfaceinterface.h"
#include "surface.h"
#include "surface_p.h"

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

}
