/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "output.h"
#include "output_p.h"
#include "region.h"
#include "surface/surface.h"
#include "surface/surfacebuffer.h"
#include "surface/surfaceevent.h"
#include "surface/surfaceview.h"

#include "qwlsurface_p.h"
#include "qwlcompositor_p.h"
#include "qwlinputdevice_p.h"
#include "qwlextendedsurface_p.h"
#include "qwlsubsurface_p.h"

#include <QtCore/QDebug>
#include <QTouchEvent>
#include <QGuiApplication>
#include <QScreen>

#include <wayland-server.h>

using namespace GreenIsland;

namespace GreenIsland {

class FrameCallback {
public:
    FrameCallback(WlSurface *surf, wl_resource *res)
        : surface(surf)
        , resource(res)
        , canSend(false)
    {
#if WAYLAND_VERSION_MAJOR < 1 || (WAYLAND_VERSION_MAJOR == 1 && WAYLAND_VERSION_MINOR <= 2)
        res->data = this;
        res->destroy = destroyCallback;
#else
        wl_resource_set_implementation(res, 0, this, destroyCallback);
#endif
    }
    ~FrameCallback()
    {
    }
    void destroy()
    {
        if (resource)
            wl_resource_destroy(resource);
        else
            delete this;
    }
    void send(uint time)
    {
        wl_callback_send_done(resource, time);
        wl_resource_destroy(resource);
    }
    static void destroyCallback(wl_resource *res)
    {
#if WAYLAND_VERSION_MAJOR < 1 || (WAYLAND_VERSION_MAJOR == 1 && WAYLAND_VERSION_MINOR <= 2)
        FrameCallback *_this = static_cast<FrameCallback *>(res->data);
#else
        FrameCallback *_this = static_cast<FrameCallback *>(wl_resource_get_user_data(res));
#endif
        _this->surface->removeFrameCallback(_this);
        delete _this;
    }
    WlSurface *surface;
    wl_resource *resource;
    bool canSend;
};

static QRegion infiniteRegion() {
    return QRegion(QRect(QPoint(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()),
                         QPoint(std::numeric_limits<int>::max(), std::numeric_limits<int>::max())));
}

WlSurface::WlSurface(struct wl_client *client, uint32_t id, int version, AbstractCompositor *compositor, Surface *surface)
    : QtWaylandServer::wl_surface(client, id, version)
    , roleHandler(Q_NULLPTR)
    , m_compositor(compositor->handle())
    , m_waylandSurface(surface)
    , m_mainOutput(0)
    , m_buffer(0)
    , m_surfaceMapped(false)
    , m_attacher(0)
    , m_extendedSurface(0)
    , m_subSurface(0)
    , m_inputPanelSurface(0)
    , m_inputRegion(infiniteRegion())
    , m_transientParent(0)
    , m_transientInactive(false)
    , m_transientOffset(QPointF(0, 0))
    , m_isCursorSurface(false)
    , m_destroyed(false)
    , m_contentOrientation(Qt::PrimaryOrientation)
    , m_visibility(QWindow::Hidden)
{
    m_pending.buffer = 0;
    m_pending.newlyAttached = false;
    m_pending.inputRegion = infiniteRegion();
}

WlSurface::~WlSurface()
{
    delete m_subSurface;

    m_bufferRef = BufferRef();

    for (int i = 0; i < m_bufferPool.size(); i++)
        m_bufferPool[i]->setDestroyIfUnused(true);

    foreach (FrameCallback *c, m_pendingFrameCallbacks)
        c->destroy();
    foreach (FrameCallback *c, m_frameCallbacks)
        c->destroy();
}

void WlSurface::setTransientOffset(qreal x, qreal y)
{
    m_transientOffset.setX(x);
    m_transientOffset.setY(y);
}

void WlSurface::releaseSurfaces()
{

}

WlSurface *WlSurface::fromResource(struct ::wl_resource *resource)
{
    return static_cast<WlSurface *>(Resource::fromResource(resource)->surface_object);
}

Surface::Type WlSurface::type() const
{
    if (m_buffer && m_buffer->waylandBufferHandle()) {
        if (m_buffer->isShmBuffer()) {
            return Surface::Shm;
        } else {
            return Surface::Texture;
        }
    }
    return Surface::Invalid;
}

bool WlSurface::isYInverted() const
{
    if (m_buffer)
        return m_buffer->isYInverted();
    return false;
}

bool WlSurface::mapped() const
{
    return !m_unmapLocks.isEmpty() || (m_buffer && bool(m_buffer->waylandBufferHandle()));
}

QSize WlSurface::size() const
{
    return m_size;
}

void WlSurface::setSize(const QSize &size)
{
    if (size != m_size) {
        m_opaqueRegion = QRegion();
        m_size = size;
        m_waylandSurface->sizeChanged();
    }
}

QRegion WlSurface::inputRegion() const
{
    return m_inputRegion;
}

QRegion WlSurface::opaqueRegion() const
{
    return m_opaqueRegion;
}

void WlSurface::sendFrameCallback()
{
    uint time = m_compositor->currentTimeMsecs();
    foreach (FrameCallback *callback, m_frameCallbacks) {
        if (callback->canSend) {
            callback->send(time);
            m_frameCallbacks.removeOne(callback);
        }
    }
}

void WlSurface::removeFrameCallback(FrameCallback *callback)
{
    m_pendingFrameCallbacks.removeOne(callback);
    m_frameCallbacks.removeOne(callback);
}

Surface * WlSurface::waylandSurface() const
{
    return m_waylandSurface;
}

QPoint WlSurface::lastMousePos() const
{
    return m_lastLocalMousePos;
}

void WlSurface::setExtendedSurface(WlExtendedSurface *extendedSurface)
{
    m_extendedSurface = extendedSurface;
    if (m_extendedSurface)
        emit m_waylandSurface->extendedSurfaceReady();
}

WlExtendedSurface *WlSurface::extendedSurface() const
{
    return m_extendedSurface;
}

void WlSurface::setSubSurface(SubSurface *subSurface)
{
    m_subSurface = subSurface;
}

SubSurface *WlSurface::subSurface() const
{
    return m_subSurface;
}

void WlSurface::addSubSurface(SubSurface *ss)
{
    m_subsurfaces << ss;
}

void WlSurface::removeSubSurface(SubSurface *ss)
{
    for (QVector<SubSurface *>::iterator i = m_subsurfaces.begin(); i != m_subsurfaces.end(); ++i) {
        if (*i == ss) {
            m_subsurfaces.erase(i);
            return;
        }
    }
}

void WlSurface::setInputPanelSurface(WlInputPanelSurface *inputPanelSurface)
{
    m_inputPanelSurface = inputPanelSurface;
}

WlInputPanelSurface *WlSurface::inputPanelSurface() const
{
    return m_inputPanelSurface;
}

WlCompositor *WlSurface::compositor() const
{
    return m_compositor;
}

Output *WlSurface::mainOutput() const
{
    if (!m_mainOutput)
        return m_compositor->primaryOutput();
    return m_mainOutput;
}

void WlSurface::setMainOutput(Output *output)
{
    m_mainOutput = output;
}

QList<Output *> WlSurface::outputs() const
{
    return m_outputs;
}

void WlSurface::addToOutput(Output *output)
{
    if (!output)
        return;

    if (!m_mainOutput)
        m_mainOutput = output;

    if (m_outputs.contains(output))
        return;

    m_outputs.append(output);

    SurfaceEnterEvent event(output);
    QCoreApplication::sendEvent(waylandSurface(), &event);

    // Send surface enter event
    Q_FOREACH (Resource *resource, resourceMap().values()) {
        QList<OutputPrivate::Resource *> outputs = output->d_func()->resourceMap().values();
        for (int i = 0; i < outputs.size(); i++)
            send_enter(resource->handle, outputs.at(i)->handle);
    }
}

void WlSurface::removeFromOutput(Output *output)
{
    if (!output)
        return;

    m_outputs.removeOne(output);

    if (m_outputs.size() == 0)
        m_mainOutput = m_compositor->primaryOutput();

    SurfaceLeaveEvent event(output);
    QCoreApplication::sendEvent(waylandSurface(), &event);

    // Send surface leave event
    Q_FOREACH (Resource *resource, resourceMap().values()) {
        QList<OutputPrivate::Resource *> outputs = output->d_func()->resourceMap().values();
        for (int i = 0; i < outputs.size(); i++)
            send_leave(resource->handle, outputs.at(i)->handle);
    }
}

/*!
 * Sets the backbuffer for this surface. The back buffer is not yet on
 * screen and will become live during the next swapBuffers().
 *
 * The backbuffer represents the current state of the surface for the
 * purpose of GUI-thread accessible properties such as size and visibility.
 */
void WlSurface::setBackBuffer(SurfaceBuffer *buffer)
{
    m_buffer = buffer;

    if (m_buffer) {
        bool valid = m_buffer->waylandBufferHandle() != 0;
        if (valid)
            setSize(m_buffer->size());

        m_damage = m_damage.intersected(QRect(QPoint(), m_size));
        emit m_waylandSurface->damaged(m_damage);
    } else {
        m_compositor->resetInputDevice(this);
    }
    m_damage = QRegion();
}

void WlSurface::setMapped(bool mapped)
{
    if (!m_surfaceMapped && mapped) {
        m_surfaceMapped = true;
        emit m_waylandSurface->mapped();
    } else if (!mapped && m_surfaceMapped) {
        m_surfaceMapped = false;
        emit m_waylandSurface->unmapped();
    }
}

void WlSurface::addUnmapLock(SurfaceUnmapLock *l)
{
    m_unmapLocks << l;
}

void WlSurface::removeUnmapLock(SurfaceUnmapLock *l)
{
    m_unmapLocks.removeOne(l);
    if (!mapped() && m_attacher) {
        setSize(QSize());
        m_attacher->unmap();
    }
}

SurfaceBuffer *WlSurface::createSurfaceBuffer(struct ::wl_resource *buffer)
{
    SurfaceBuffer *newBuffer = 0;
    for (int i = 0; i < m_bufferPool.size(); i++) {
        if (!m_bufferPool[i]->isRegisteredWithBuffer()) {
            newBuffer = m_bufferPool[i];
            newBuffer->initialize(buffer);
            break;
        }
    }

    if (!newBuffer) {
        newBuffer = new SurfaceBuffer(waylandSurface());
        newBuffer->initialize(buffer);
        m_bufferPool.append(newBuffer);
        if (m_bufferPool.size() > 3)
            qWarning() << "Increased buffer pool size to" << m_bufferPool.size() << "for surface with title:" << title() << "className:" << className();
    }

    return newBuffer;
}

Qt::ScreenOrientation WlSurface::contentOrientation() const
{
    return m_contentOrientation;
}

void WlSurface::surface_destroy_resource(Resource *)
{
    if (m_extendedSurface) {
        m_extendedSurface->setParentSurface(Q_NULLPTR);
        m_extendedSurface = 0;
    }

    if (transientParent()) {
        foreach (WlSurface *surface, compositor()->surfaces()) {
            if (surface->transientParent() == this) {
                surface->setTransientParent(0);
            }
        }
    }

    m_destroyed = true;
    m_waylandSurface->destroy();
    emit m_waylandSurface->surfaceDestroyed();
}

void WlSurface::surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WlSurface::surface_attach(Resource *, struct wl_resource *buffer, int x, int y)
{
    if (m_pending.buffer)
        m_pending.buffer->disown();
    m_pending.buffer = createSurfaceBuffer(buffer);
    m_pending.offset = QPoint(x, y);
    m_pending.newlyAttached = true;
}

void WlSurface::surface_damage(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    m_pending.damage = m_pending.damage.united(QRect(x, y, width, height));
}

void WlSurface::surface_frame(Resource *resource, uint32_t callback)
{
    struct wl_resource *frame_callback = wl_resource_create(resource->client(), &wl_callback_interface, wl_callback_interface.version, callback);
    m_pendingFrameCallbacks << new FrameCallback(this, frame_callback);
}

void WlSurface::surface_set_opaque_region(Resource *, struct wl_resource *region)
{
    m_opaqueRegion = region ? Region::fromResource(region)->region() : QRegion();
}

void WlSurface::surface_set_input_region(Resource *, struct wl_resource *region)
{
    if (region) {
        m_pending.inputRegion = Region::fromResource(region)->region();
    } else {
        m_pending.inputRegion = infiniteRegion();
    }
}

void WlSurface::surface_commit(Resource *)
{
    m_damage = m_pending.damage;

    if (m_pending.buffer || m_pending.newlyAttached) {
        setBackBuffer(m_pending.buffer);
        m_bufferRef = BufferRef(m_buffer);

        if (m_attacher) {
            if (m_bufferRef) {
                m_attacher->attach(m_bufferRef);
            } else if (!mapped()) {
                setSize(QSize());
                m_attacher->unmap();
            }
        }
        emit m_waylandSurface->configure(m_bufferRef);
        if (roleHandler)
            roleHandler->configure(m_pending.offset.x(), m_pending.offset.y());
    }

    m_pending.buffer = 0;
    m_pending.offset = QPoint();
    m_pending.newlyAttached = false;
    m_pending.damage = QRegion();

    foreach (SubSurface *ss, m_subsurfaces)
        ss->parentCommit();

    if (m_buffer)
        m_buffer->setCommitted();

    m_frameCallbacks << m_pendingFrameCallbacks;
    m_pendingFrameCallbacks.clear();

    m_inputRegion = m_pending.inputRegion.intersected(QRect(QPoint(), m_size));

    emit m_waylandSurface->redraw();
}

void WlSurface::surface_set_buffer_transform(Resource *resource, int32_t orientation)
{
    Q_UNUSED(resource);
    QScreen *screen = QGuiApplication::primaryScreen();
    bool isPortrait = screen->primaryOrientation() == Qt::PortraitOrientation;
    Qt::ScreenOrientation oldOrientation = m_contentOrientation;
    switch (orientation) {
        case WL_OUTPUT_TRANSFORM_90:
            m_contentOrientation = isPortrait ? Qt::InvertedLandscapeOrientation : Qt::PortraitOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_180:
            m_contentOrientation = isPortrait ? Qt::InvertedPortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_270:
            m_contentOrientation = isPortrait ? Qt::LandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        default:
            m_contentOrientation = Qt::PrimaryOrientation;
    }
    if (m_contentOrientation != oldOrientation)
        emit waylandSurface()->contentOrientationChanged();
}

void WlSurface::frameStarted()
{
    foreach (FrameCallback *c, m_frameCallbacks)
        c->canSend = true;
}

void WlSurface::setClassName(const QString &className)
{
    if (m_className != className) {
        m_className = className;
        emit waylandSurface()->classNameChanged();
    }
}

void WlSurface::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit waylandSurface()->titleChanged();
    }
}

} // namespace Wayland

