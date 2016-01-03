/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "buffer_p.h"
#include "output_p.h"
#include "region.h"
#include "region_p.h"
#include "registry.h"
#include "registry_p.h"
#include "surface.h"
#include "surface_p.h"

namespace GreenIsland {

namespace Client {

static QtWayland::wl_output::transform toWlTransform(const Output::Transform &value)
{
    switch (value) {
    case Output::Transform90:
        return QtWayland::wl_output::transform_90;
    case Output::Transform180:
        return QtWayland::wl_output::transform_180;
    case Output::Transform270:
        return QtWayland::wl_output::transform_270;
    case Output::TransformFlipped:
        return QtWayland::wl_output::transform_flipped;
    case Output::TransformFlipped90:
        return QtWayland::wl_output::transform_flipped_90;
    case Output::TransformFlipped180:
        return QtWayland::wl_output::transform_flipped_180;
    case Output::TransformFlipped270:
        return QtWayland::wl_output::transform_flipped_270;
    default:
        break;
    }

    return QtWayland::wl_output::transform_normal;
}

/*
 * SurfacePrivate
 */

SurfacePrivate::SurfacePrivate()
    : QtWayland::wl_surface()
    , frameCallbackInstalled(false)
{
}

void SurfacePrivate::handleFrameCallback()
{
    Q_Q(Surface);
    frameCallbackInstalled = true;
    Q_EMIT q->frameRendered();
}

void SurfacePrivate::frameCallback(void *data, wl_callback *callback, uint32_t time)
{
    Q_UNUSED(time);

    SurfacePrivate *that = reinterpret_cast<SurfacePrivate *>(data);
    Q_ASSERT(that);
    if (callback)
        wl_callback_destroy(callback);
    that->handleFrameCallback();
}

Surface *SurfacePrivate::fromWlSurface(struct ::wl_surface *surface)
{
    QtWayland::wl_surface *wlSurface =
            static_cast<QtWayland::wl_surface *>(wl_surface_get_user_data(surface));
    return static_cast<SurfacePrivate *>(wlSurface)->q_func();
}

void SurfacePrivate::surface_enter(wl_output *output)
{
    Q_Q(Surface);
    Q_EMIT q->enter(OutputPrivate::fromWlOutput(output));
}

void SurfacePrivate::surface_leave(wl_output *output)
{
    Q_Q(Surface);
    Q_EMIT q->leave(OutputPrivate::fromWlOutput(output));
}

const struct wl_callback_listener SurfacePrivate::callbackListener = {
    frameCallback
};

/*
 * Surface
 */

Surface::Surface(QObject *parent)
    : QObject(*new SurfacePrivate(), parent)
{
}

void Surface::setupFrameCallback()
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    wl_callback *callback = d->frame();
    wl_callback_add_listener(callback, &d->callbackListener, d);
    d->frameCallbackInstalled = true;
}

void Surface::attach(Buffer *buffer, const QPoint &position)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    if (buffer)
        d->attach(BufferPrivate::get(buffer)->object(),
                  position.x(), position.y());
    else
        d->attach(Q_NULLPTR, 0, 0);
}

void Surface::attach(BufferPtr buffer, const QPoint &position)
{
    attach(buffer.toStrongRef().data(), position);
}

void Surface::damage(const QRect &rect)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    d->damage(rect.x(), rect.y(), rect.width(), rect.height());
}

void Surface::commit(CommitMode mode)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    if (mode == FrameCallbackCommitMode)
        setupFrameCallback();
    d->commit();
}

void Surface::setOpaqueRegion(Region *region)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    d->set_opaque_region(region ? RegionPrivate::get(region)->object() : Q_NULLPTR);
}

void Surface::setInputRegion(Region *region)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    d->set_input_region(region ? RegionPrivate::get(region)->object() : Q_NULLPTR);
}

void Surface::setBufferTransform(Output::Transform transform)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    d->set_buffer_transform(toWlTransform(transform));
}

void Surface::setBufferScale(qint32 scale)
{
    Q_D(Surface);

    if (!d->isInitialized())
        return;

    d->set_buffer_scale(scale);
}

Surface *Surface::fromQt(QWindow *window, QObject *parent)
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland")))
        return Q_NULLPTR;

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (!native)
        return Q_NULLPTR;

    wl_surface *surface = reinterpret_cast<wl_surface *>(
                native->nativeResourceForWindow(QByteArrayLiteral("surface"), window));
    if (!surface)
        return Q_NULLPTR;

    Surface *s = new Surface(parent);
    SurfacePrivate::get(s)->init(surface);
    return s;
}

QByteArray Surface::interfaceName()
{
    return QByteArrayLiteral("wl_surface");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_surface.cpp"
