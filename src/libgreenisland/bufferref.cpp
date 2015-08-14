/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Giulio Camuffo
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#include <QtCore/QAtomicInt>

#include "bufferref.h"
#include "surfacebuffer.h"

namespace GreenIsland {

/*
 * BufferRef
 */

class BufferRefPrivate
{
public:
    SurfaceBuffer *buffer;
};

/*
 * BufferRef
 */

BufferRef::BufferRef()
    : d(new BufferRefPrivate)
{
    d->buffer = Q_NULLPTR;
}

BufferRef::BufferRef(SurfaceBuffer *buffer)
    : d(new BufferRefPrivate)
{
    d->buffer = buffer;
    if (buffer)
        buffer->ref();
}

BufferRef::BufferRef(const BufferRef &ref)
    : d(new BufferRefPrivate)
{
    d->buffer = Q_NULLPTR;
    *this = ref;
}

BufferRef::~BufferRef()
{
    if (d->buffer)
        d->buffer->deref();
    delete d;
}

BufferRef &BufferRef::operator=(const BufferRef &ref)
{
    if (d->buffer)
        d->buffer->deref();

    d->buffer = ref.d->buffer;
    if (d->buffer)
        d->buffer->ref();

    return *this;
}

BufferRef::operator bool() const
{
    return d->buffer && d->buffer->waylandBufferHandle();
}

bool BufferRef::isShm() const
{
    return d->buffer->isShmBuffer();
}

QImage BufferRef::image() const
{
    if (d->buffer->isShmBuffer())
        return d->buffer->image();
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL

GLuint BufferRef::createTexture()
{
    if (!d->buffer->isShmBuffer() && !d->buffer->textureCreated()) {
        d->buffer->createTexture();
    }
    return d->buffer->texture();
}

void BufferRef::destroyTexture()
{
    if (!d->buffer->isShmBuffer() && d->buffer->textureCreated()) {
        d->buffer->destroyTexture();
    }
}

void *BufferRef::nativeBuffer() const
{
    return d->buffer->handle();
}

#endif // QT_COMPOSITOR_WAYLAND_GL

} // namespace GreenIsland
