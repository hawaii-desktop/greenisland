/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QSGTexture>
#include <QOpenGLTexture>
#include <QQuickWindow>
#include <QDebug>
#include <QQmlPropertyMap>

#include "quicksurface.h"
#include "abstractquickcompositor.h"
#include "surfaceitem.h"
#include "bufferref.h"
#include "output_p.h"
#include "outputwindow.h"
#include "surface_p.h"
#include "surfaceevent.h"

namespace GreenIsland {

class BufferAttacher : public AbstractBufferAttacher
{
public:
    BufferAttacher()
        : surface(0)
        , texture(0)
        , update(false)
    {

    }

    ~BufferAttacher()
    {
        if (texture)
            texture->deleteLater();
        bufferRef = BufferRef();
        nextBuffer = BufferRef();
    }

    void attach(const BufferRef &ref) Q_DECL_OVERRIDE
    {
        nextBuffer = ref;
        update = true;
    }

    void createTexture()
    {
        bufferRef = nextBuffer;
        delete texture;
        texture = 0;

        OutputWindow *window = surface->mainOutput()->window();
        if (nextBuffer) {
            if (bufferRef.isShm()) {
                texture = window->createTextureFromImage(bufferRef.image());
            } else {
                QQuickWindow::CreateTextureOptions opt = 0;
                if (surface->useTextureAlpha()) {
                    opt |= QQuickWindow::TextureHasAlphaChannel;
                }
                texture = window->createTextureFromId(bufferRef.createTexture(), surface->size(), opt);
            }
            texture->bind();
        }

        update = false;
    }

    void unmap() Q_DECL_OVERRIDE
    {
        nextBuffer = BufferRef();
        update = true;
    }

    void invalidateTexture()
    {
        if (bufferRef)
            bufferRef.destroyTexture();
        delete texture;
        texture = 0;
        update = true;
        bufferRef = BufferRef();
    }

    QuickSurface *surface;
    BufferRef bufferRef;
    BufferRef nextBuffer;
    QSGTexture *texture;
    bool update;
};


class QuickSurfacePrivate : public SurfacePrivate
{
public:
    QuickSurfacePrivate(wl_client *client, quint32 id, int version, AbstractQuickCompositor *c, QuickSurface *surf)
        : SurfacePrivate(client, id, version, c, surf)
        , buffer(new BufferAttacher)
        , compositor(c)
        , useTextureAlpha(true)
        , windowPropertyMap(new QQmlPropertyMap)
        , clientRenderingEnabled(true)
    {

    }

    ~QuickSurfacePrivate()
    {
        windowPropertyMap->deleteLater();
        // buffer is deleted automatically by ~Surface(), since it is the assigned attacher
    }

    void surface_commit(Resource *resource) Q_DECL_OVERRIDE
    {
        SurfacePrivate::surface_commit(resource);

        Q_FOREACH (Output *output, outputs())
            output->d_func()->update();
    }

    BufferAttacher *buffer;
    AbstractQuickCompositor *compositor;
    bool useTextureAlpha;
    QQmlPropertyMap *windowPropertyMap;
    bool clientRenderingEnabled;
};

QuickSurface::QuickSurface(wl_client *client, quint32 id, int version, AbstractQuickCompositor *compositor)
                    : Surface(new QuickSurfacePrivate(client, id, version, compositor, this))
{
    Q_D(QuickSurface);
    d->buffer->surface = this;
    setBufferAttacher(d->buffer);

    connect(this, &Surface::windowPropertyChanged, d->windowPropertyMap, &QQmlPropertyMap::insert);
    connect(d->windowPropertyMap, &QQmlPropertyMap::valueChanged, this, &Surface::setWindowProperty);
}

QuickSurface::~QuickSurface()
{

}

QSGTexture *QuickSurface::texture() const
{
    Q_D(const QuickSurface);
    return d->buffer->texture;
}

bool QuickSurface::useTextureAlpha() const
{
    Q_D(const QuickSurface);
    return d->useTextureAlpha;
}

void QuickSurface::setUseTextureAlpha(bool useTextureAlpha)
{
    Q_D(QuickSurface);
    if (d->useTextureAlpha != useTextureAlpha) {
        d->useTextureAlpha = useTextureAlpha;
        emit useTextureAlphaChanged();
        emit configure(d->buffer->bufferRef);
    }
}

QObject *QuickSurface::windowPropertyMap() const
{
    Q_D(const QuickSurface);
    return d->windowPropertyMap;
}

bool QuickSurface::event(QEvent *e)
{
    if (e->type() == static_cast<QEvent::Type>(SurfaceLeaveEvent::WaylandSurfaceLeave)) {
        SurfaceLeaveEvent *event = static_cast<SurfaceLeaveEvent *>(e);

        if (event->output()) {
            QQuickWindow *oldWindow = static_cast<QQuickWindow *>(event->output()->window());
            disconnect(oldWindow, &QQuickWindow::beforeSynchronizing,
                       this, &QuickSurface::updateTexture);
            disconnect(oldWindow, &QQuickWindow::sceneGraphInvalidated,
                       this, &QuickSurface::invalidateTexture);
            disconnect(oldWindow, &QQuickWindow::sceneGraphAboutToStop,
                       this, &QuickSurface::invalidateTexture);
        }

        return true;
    }

    if (e->type() == static_cast<QEvent::Type>(SurfaceEnterEvent::WaylandSurfaceEnter)) {
        SurfaceEnterEvent *event = static_cast<SurfaceEnterEvent *>(e);

        if (event->output()) {
            QQuickWindow *window = static_cast<QQuickWindow *>(event->output()->window());
            connect(window, &QQuickWindow::beforeSynchronizing,
                    this, &QuickSurface::updateTexture,
                    Qt::DirectConnection);
            connect(window, &QQuickWindow::sceneGraphInvalidated,
                    this, &QuickSurface::invalidateTexture,
                    Qt::DirectConnection);
            connect(window, &QQuickWindow::sceneGraphAboutToStop,
                    this, &QuickSurface::invalidateTexture,
                    Qt::DirectConnection);
        }

        return true;
    }

    return QObject::event(e);
}

void QuickSurface::updateTexture()
{
    Q_D(QuickSurface);
    const bool update = d->buffer->update;
    if (d->buffer->update)
        d->buffer->createTexture();
    foreach (SurfaceView *view, views())
        static_cast<SurfaceItem *>(view)->updateTexture(update);
}

void QuickSurface::invalidateTexture()
{
    Q_D(QuickSurface);
    d->buffer->invalidateTexture();
    foreach (SurfaceView *view, views())
        static_cast<SurfaceItem *>(view)->updateTexture(true);
    emit redraw();
}

bool QuickSurface::clientRenderingEnabled() const
{
    Q_D(const QuickSurface);
    return d->clientRenderingEnabled;
}

void QuickSurface::setClientRenderingEnabled(bool enabled)
{
    Q_D(QuickSurface);
    if (d->clientRenderingEnabled != enabled) {
        d->clientRenderingEnabled = enabled;

        sendOnScreenVisibilityChange(enabled);

        emit clientRenderingEnabledChanged();
    }
}

}
