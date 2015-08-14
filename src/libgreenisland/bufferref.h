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

#ifndef GREENISLAND_BUFFERREF_H
#define GREENISLAND_BUFFERREF_H

#include <QtGui/QImage>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/qopengl.h>
#endif

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class SurfaceBuffer;

class GREENISLAND_EXPORT BufferRef
{
public:
    BufferRef();
    explicit BufferRef(SurfaceBuffer *buffer);
    BufferRef(const BufferRef &ref);
    ~BufferRef();

    BufferRef &operator=(const BufferRef &ref);
    operator bool() const;
    bool isShm() const;

    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    /**
     * There must be a GL context bound when calling this function.
     * The texture will be automatically destroyed when the last BufferRef
     * referring to the same underlying buffer will be destroyed or reset.
     */
    GLuint createTexture();
    void destroyTexture();
    void *nativeBuffer() const;
#endif

private:
    class BufferRefPrivate *const d;
    friend class BufferRefPrivate;
};

} // namespace GreenIsland

#endif // GREENISLAND_BUFFERREF_H
