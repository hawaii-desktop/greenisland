/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "surface.h"

#include <private/qobject_p.h>

#include "wayland_wrapper/qwlsurface_p.h"
#include "wayland_wrapper/qwlextendedsurface_p.h"
#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwlinputdevice_p.h"
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwldatadevicemanager_p.h"

#include "abstractcompositor.h"
#include "clientconnection.h"
#include "surface_p.h"
#include "bufferref.h"
#include "surfaceinterface.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

namespace GreenIsland {

SurfacePrivate::SurfacePrivate(wl_client *wlClient, quint32 id, int version, AbstractCompositor *compositor, Surface *surface)
    : GreenIsland::WlSurface(wlClient, id, version, compositor, surface)
    , closing(false)
    , refCount(1)
    , client(ClientConnection::fromWlClient(wlClient))
    , windowType(Surface::None)
{}


Surface::Surface(wl_client *client, quint32 id, int version, AbstractCompositor *compositor)
    : QObject(*new SurfacePrivate(client, id, version, compositor, this))
{

}

Surface::Surface(SurfacePrivate *dptr)
    : QObject(*dptr)
{

}

Surface::~Surface()
{
    Q_D(Surface);
    qDeleteAll(d->interfaces);
    delete d->m_attacher;
}

ClientConnection *Surface::client() const
{
    Q_D(const Surface);
    if (d->isDestroyed() || !d->compositor()->clients().contains(d->client))
        return Q_NULLPTR;
    return d->client;
}

void Surface::addInterface(SurfaceInterface *iface)
{
    Q_D(Surface);
    d->interfaces.prepend(iface);
}

void Surface::removeInterface(SurfaceInterface *iface)
{
    Q_D(Surface);
    d->interfaces.removeOne(iface);
}

Surface::Type Surface::type() const
{
    Q_D(const Surface);
    return d->type();
}

bool Surface::isYInverted() const
{
    Q_D(const Surface);
    return d->isYInverted();
}

bool Surface::visible() const
{
    return isMapped();
}

bool Surface::isMapped() const
{
    Q_D(const Surface);
    return d->mapped();
}

QSize Surface::size() const
{
    Q_D(const Surface);
    return d->size();
}

void Surface::requestSize(const QSize &size)
{
    Q_D(Surface);
    SurfaceResizeOperation op(size);
    if (!sendInterfaceOp(op)) {
        int id = wl_resource_get_id(d->resource()->handle);
        qWarning("No surface interface forwarded the resize request for this surface (wl_surface@%d).", id);
    }
}

Qt::ScreenOrientations Surface::orientationUpdateMask() const
{
    Q_D(const Surface);
    if (!d->extendedSurface())
        return Qt::PrimaryOrientation;
    return d->extendedSurface()->contentOrientationMask();
}

Qt::ScreenOrientation Surface::contentOrientation() const
{
    Q_D(const Surface);
    return d->contentOrientation();
}

Surface::WindowFlags Surface::windowFlags() const
{
    Q_D(const Surface);
    if (!d->extendedSurface())
        return Surface::WindowFlags(0);
    return d->extendedSurface()->windowFlags();
}

Surface::WindowType Surface::windowType() const
{
    Q_D(const Surface);
    return d->windowType;
}

Surface *Surface::transientParent() const
{
    Q_D(const Surface);
    return d->transientParent() ? d->transientParent()->waylandSurface() : 0;
}

QPointF Surface::transientOffset() const
{
    Q_D(const Surface);
    return d->m_transientOffset;
}

GreenIsland::WlSurface * Surface::handle()
{
    Q_D(Surface);
    return d;
}

QVariantMap Surface::windowProperties() const
{
    Q_D(const Surface);
    if (!d->extendedSurface())
        return QVariantMap();

    return d->extendedSurface()->windowProperties();
}

void Surface::setWindowProperty(const QString &name, const QVariant &value)
{
    Q_D(Surface);
    if (!d->extendedSurface())
        return;

    d->extendedSurface()->setWindowProperty(name, value);
}

AbstractCompositor *Surface::compositor() const
{
    Q_D(const Surface);
    return d->compositor()->waylandCompositor();
}

Output *Surface::mainOutput() const
{
    Q_D(const Surface);

    // Returns the output that contains the most if not all
    // the surface (window managers will take care of setting
    // this, defaults to the first output)
    return d->mainOutput();
}

void Surface::setMainOutput(Output *mainOutput)
{
    Q_D(Surface);

    if (mainOutput)
        d->setMainOutput(mainOutput);
}

QList<Output *> Surface::outputs() const
{
    Q_D(const Surface);

    return d->outputs();
}

QWindow::Visibility Surface::visibility() const
{
    Q_D(const Surface);
    return d->m_visibility;
}

void Surface::setVisibility(QWindow::Visibility v)
{
    Q_D(Surface);
    if (v == visibility())
        return;

    d->m_visibility = v;
    SurfaceSetVisibilityOperation op(v);
    sendInterfaceOp(op);

    emit visibilityChanged();
}

bool Surface::sendInterfaceOp(SurfaceOperation &op)
{
    Q_D(Surface);
    foreach (SurfaceInterface *iface, d->interfaces) {
        if (iface->runOperation(&op))
            return true;
    }
    return false;
}

void Surface::ping()
{
    Q_D(Surface);
    uint32_t serial = wl_display_next_serial(compositor()->waylandDisplay());
    SurfacePingOperation op(serial);
    if (!sendInterfaceOp(op)) {
        int id = wl_resource_get_id(d->resource()->handle);
        qWarning("No surface interface forwarded the ping for this surface (wl_surface@%d).", id);
    }
}

void Surface::sendOnScreenVisibilityChange(bool visible)
{
    setVisibility(visible ? QWindow::AutomaticVisibility : QWindow::Hidden);
}

QString Surface::className() const
{
    Q_D(const Surface);
    return d->className();
}

QString Surface::title() const
{
    Q_D(const Surface);
    return d->title();
}

bool Surface::hasInputPanelSurface() const
{
    Q_D(const Surface);

    return d->inputPanelSurface() != 0;
}

/*!
 * \return True if WL_SHELL_SURFACE_TRANSIENT_INACTIVE was set for this surface, meaning it should not receive keyboard focus.
 */
bool Surface::transientInactive() const
{
    Q_D(const Surface);
    return d->transientInactive();
}

bool Surface::inputRegionContains(const QPoint &p) const
{
    Q_D(const Surface);
    return d->inputRegion().contains(p);
}

void Surface::destroy()
{
    Q_D(Surface);
    if (--d->refCount == 0)
        compositor()->handle()->destroySurface(d);
}

void Surface::destroySurface()
{
    SurfaceOperation op(SurfaceOperation::Close);
    if (!sendInterfaceOp(op))
        emit surfaceDestroyed();
}

/*!
    Updates the surface with the compositor's retained clipboard selection. While this
    is done automatically when the surface receives keyboard focus, this function is
    useful for updating clients which do not have keyboard focus.
*/
void Surface::updateSelection()
{
    Q_D(Surface);
    const GreenIsland::WlInputDevice *inputDevice = d->compositor()->defaultInputDevice();
    if (inputDevice) {
        const GreenIsland::WlDataDevice *dataDevice = inputDevice->dataDevice();
        if (dataDevice) {
            d->compositor()->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->resource()->client())->handle);
        }
    }
}

void Surface::ref()
{
    Q_D(Surface);
    ++d->refCount;
}

void Surface::setMapped(bool mapped)
{
    Q_D(Surface);
    d->setMapped(mapped);
}

void Surface::setBufferAttacher(AbstractBufferAttacher *attacher)
{
    Q_D(Surface);
    d->m_attacher = attacher;
}

AbstractBufferAttacher *Surface::bufferAttacher() const
{
    Q_D(const Surface);
    return d->m_attacher;
}

QList<SurfaceView *> Surface::views() const
{
    Q_D(const Surface);
    return d->views;
}

QList<SurfaceInterface *> Surface::interfaces() const
{
    Q_D(const Surface);
    return d->interfaces;
}

Surface *Surface::fromResource(::wl_resource *res)
{
    GreenIsland::WlSurface *s = GreenIsland::WlSurface::fromResource(res);
    if (s)
        return s->waylandSurface();
    return Q_NULLPTR;
}

void SurfacePrivate::setTitle(const QString &title)
{
    Q_Q(Surface);
    if (m_title != title) {
        m_title = title;
        emit q->titleChanged();
    }
}

void SurfacePrivate::setClassName(const QString &className)
{
    Q_Q(Surface);
    if (m_className != className) {
        m_className = className;
        emit q->classNameChanged();
    }
}

void SurfacePrivate::setType(Surface::WindowType type)
{
    Q_Q(Surface);
    if (windowType != type) {
        windowType = type;
        emit q->windowTypeChanged(type);
    }
}

class SurfaceUnmapLockPrivate
{
public:
    Surface *surface;
};

/*!
    Constructs a UnmapLock object.

    The lock will act on the \a surface parameter, and will prevent the surface to
    be unmapped, retaining the last valid buffer when the client attachs a NULL buffer.
    The lock will be automatically released when deleted.
*/
SurfaceUnmapLock::SurfaceUnmapLock(Surface *surface)
    : d(new SurfaceUnmapLockPrivate)
{
    d->surface = surface;
    surface->handle()->addUnmapLock(this);
}

SurfaceUnmapLock::~SurfaceUnmapLock()
{
    d->surface->handle()->removeUnmapLock(this);
    delete d;
}

}
