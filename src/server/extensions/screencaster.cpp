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

#include <QtCore/QCoreApplication>
#include <QtCore/QMutexLocker>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickItemGrabResult>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandDestroyListener>

#include "screencaster.h"
#include "screencaster_p.h"
#include "serverlogging_p.h"

#define SCREENCASTER_FORMAT WL_SHM_FORMAT_XRGB8888

namespace GreenIsland {

namespace Server {

static const QEvent::Type SuccessEventType =
        static_cast<QEvent::Type>(QEvent::registerEventType());
static const QEvent::Type FailedEventType =
        static_cast<QEvent::Type>(QEvent::registerEventType());

class SuccessEvent : public QEvent
{
public:
    SuccessEvent(uint t)
        : QEvent(SuccessEventType)
        , time(t)
    {
    }

    uint time;
};

class FailedEvent : public QEvent
{
public:
    enum Error {
        BadBuffer = 0
    };

    FailedEvent(Error e)
        : QEvent(FailedEventType)
        , error(e)
    {
    }

    Error error;
};

/*
 * ScreencasterPrivate
 */

ScreencasterPrivate::ScreencasterPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_screencaster()
{
}

void ScreencasterPrivate::addRequest(QQuickWindow *window, Screencast *screencast)
{
    QMutexLocker locker(&requestsMutex);
    requests.insert(window, screencast);
}

void ScreencasterPrivate::removeRequest(QQuickWindow *window, Screencast *screencast)
{
    QMutexLocker locker(&requestsMutex);
    if (window) {
        requests.remove(window, screencast);
    } else {
        for (auto it = requests.begin(); it != requests.end();) {
            if (it.value() == screencast)
                it = requests.erase(it);
            else
                ++it;
        }
    }
}

void ScreencasterPrivate::screencaster_bind_resource(Resource *resource)
{
    // TODO: Deny access to unauthorized clients
    QtWaylandServer::greenisland_screencaster::screencaster_bind_resource(resource);
}

void ScreencasterPrivate::screencaster_destroy_resource(Resource *resource)
{
    // A client disconnected, invalidate all its screencast requests
    QtWaylandServer::greenisland_screencaster::screencaster_destroy_resource(resource);
}

void ScreencasterPrivate::screencaster_capture(Resource *resource, uint32_t id,
                                               struct ::wl_resource *outputResource)
{
    Q_Q(Screencaster);

    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    if (!output) {
        qCWarning(gLcScreencaster) << "Couldn't find output from resource";
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "the specified output is invalid");
        return;
    }

    QQuickWindow *window = static_cast<QQuickWindow *>(output->window());
    if (!window) {
        qCWarning(gLcScreencaster) << "Output has no QtQuick window";
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "the specified output doesn't have a QtQuick window");
        return;
    }

    Screencast *screencast = new Screencast();
    ScreencastPrivate *dScreencast = ScreencastPrivate::get(screencast);
    dScreencast->screencaster = q;
    dScreencast->output = output;
    dScreencast->window = window;
    screencast->setExtensionContainer(q);
    screencast->initialize();
    dScreencast->init(resource->client(), id, 1);

    // Tell the client to allocate a buffer as big as the output
    dScreencast->send_setup(window->width(), window->height(),
                            window->width() * 4, SCREENCASTER_FORMAT);

    // Emit a signal for the capture request
    Q_EMIT q->captureRequested(screencast);
}

/*
 * Screencaster
 */

Screencaster::Screencaster()
    : QWaylandExtensionTemplate<Screencaster>(*new ScreencasterPrivate())
{
}

Screencaster::Screencaster(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<Screencaster>(compositor, *new ScreencasterPrivate())
{
}

void Screencaster::initialize()
{
    Q_D(Screencaster);

    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcScreencaster) << "Failed to find QWaylandCompositor when initializing Screencaster";
        return;
    }
    d->init(compositor->display(), 1);
}

void Screencaster::recordFrame(QQuickWindow *window)
{
    Q_D(Screencaster);

    // NOTE: This must be called from the rendering thread
    // otherwise grabWindow() won't work

    // Serialize access to the requests map
    QMutexLocker locker(&d->requestsMutex);

    // Just return if there are no record requests left
    if (d->requests.isEmpty())
        return;

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    Q_ASSERT(compositor);

    uint time = compositor->currentTimeMsecs();

    // Satisfy all pending record requests
    Q_FOREACH (Screencast *screencast, d->requests.values(window)) {
        // Remove request
        d->requests.remove(window, screencast);

        // Get buffer
        if (!ScreencastPrivate::get(screencast)->buffer)
            continue;

        // Grab a picture
#if 1
        QImage image = window->grabWindow();
        ScreencastPrivate::get(screencast)->frameRecording(screencast, time, image);
#else
        QSharedPointer<QQuickItemGrabResult> result = window->contentItem()->grabToImage();
        if (!result.isNull() && !result->image().isNull()) {
            ScreencastPrivate::get(screencast)->frameRecording(screencast, time,
                                                               result->image());
        } else {
            connect(result.data(), &QQuickItemGrabResult::ready, this, [this, screencast, time] {
                QQuickItemGrabResult *result = static_cast<QQuickItemGrabResult *>(sender());
                if (result)
                    ScreencastPrivate::get(screencast)->frameRecording(screencast, time,
                                                                       result->image());
            }, Qt::QueuedConnection);
        }
#endif
    }
}

const struct wl_interface *Screencaster::interface()
{
    return ScreencasterPrivate::interface();
}

QByteArray Screencaster::interfaceName()
{
    return ScreencasterPrivate::interfaceName();
}

/*
 * ScreencastPrivate
 */

ScreencastPrivate::ScreencastPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_screencast()
    , valid(true)
    , screencaster(Q_NULLPTR)
    , output(Q_NULLPTR)
    , window(Q_NULLPTR)
    , bufferResource(Q_NULLPTR)
    , buffer(Q_NULLPTR)
{
}

void ScreencastPrivate::frameRecording(Screencast *screencast,
                                       uint time, const QImage &image)
{
    // Skip recording if the request was just deleted - this might
    // happen when the client quits and sends screencast_destroy
    if (!valid)
        return;

    // Buffer data
    uchar *data = static_cast<uchar *>(wl_shm_buffer_get_data(buffer));
    int32_t height = wl_shm_buffer_get_height(buffer);
    int32_t stride = wl_shm_buffer_get_stride(buffer);

    // Verify buffer size against image size
    if (image.byteCount() != stride * height) {
        QCoreApplication::postEvent(screencast, new FailedEvent(FailedEvent::BadBuffer));
        qCWarning(gLcScreencaster, "Bad record frame request: buffer is %d bytes, frame is %d bytes",
                  stride * height, image.byteCount());
    } else {
        memcpy(data, image.bits(), stride * height);
        QCoreApplication::postEvent(screencast, new SuccessEvent(SuccessEvent(time)));
    }
}

void ScreencastPrivate::screencast_destroy(Resource *resource)
{
    Q_UNUSED(resource);

    Q_Q(Screencast);

    // Invalidate the object
    valid = false;

    // Remove the request
    ScreencasterPrivate::get(screencaster)->removeRequest(Q_NULLPTR, q);

    // Delete
    q->deleteLater();
}

void ScreencastPrivate::screencast_record(Resource *resource,
                                          struct ::wl_resource *br)
{
    Q_UNUSED(resource);

    Q_Q(Screencast);

    // Listen for destruction to avoid using a buffer that has been already
    // destroyed (for example when the client exits without a proper delete sequence)
    QWaylandDestroyListener *listener = new QWaylandDestroyListener();
    listener->listenForDestruction(br);
    QObject::connect(listener, &QWaylandDestroyListener::fired, [this, listener](void *data) {
        if (data == bufferResource) {
            bufferResource = Q_NULLPTR;
            buffer = Q_NULLPTR;
        }
        delete listener;
    });

    // If we already have a pointer to a buffer resource, it means that
    // previously we received a record request thus we need to cancel
    if (bufferResource)
        send_cancelled(bufferResource);

    // Save a pointer to the frame buffer
    bufferResource = br;

    // Verify the buffer
    buffer = wl_shm_buffer_get(br);
    if (buffer) {
        int32_t w = wl_shm_buffer_get_width(buffer);
        int32_t h = wl_shm_buffer_get_height(buffer);
        int32_t s = wl_shm_buffer_get_stride(buffer);
        uint32_t f = wl_shm_buffer_get_format(buffer);

        if (w < output->geometry().width() ||
                h < output->geometry().height()) {
            qCWarning(gLcScreencaster, "Buffer size %dx%d doesn't match output size %dx%d",
                      w, h, output->geometry().width(), output->geometry().height());
            buffer = Q_NULLPTR;
        }
        if (s != w * 4) {
            qCWarning(gLcScreencaster, "Buffer stride %d doesn't match calculated stride %d",
                      s, w * 4);
            buffer = Q_NULLPTR;
        }
        if (f != SCREENCASTER_FORMAT) {
            qCWarning(gLcScreencaster, "Wrong buffer format %d, expected %d",
                      f, SCREENCASTER_FORMAT);
            buffer = Q_NULLPTR;
        }
    }
    if (!buffer) {
        send_failed(error_bad_buffer, bufferResource);
        bufferResource = Q_NULLPTR;
        return;
    }

    // Schedule the record request
    ScreencasterPrivate::get(screencaster)->addRequest(window, q);
}

/*
 * Screencast
 */

Screencast::Screencast()
    : QWaylandExtensionTemplate<Screencast>(*new ScreencastPrivate())
{
}

Screencast::~Screencast()
{
    Q_D(Screencast);
    ScreencasterPrivate::get(d->screencaster)->removeRequest(d->window, this);
}

const struct wl_interface *Screencast::interface()
{
    return ScreencastPrivate::interface();
}

QByteArray Screencast::interfaceName()
{
    return ScreencastPrivate::interfaceName();
}

bool Screencast::event(QEvent *event)
{
    Q_D(Screencast);

    // Send events unless the client has already destroyed us
    if (d->valid) {
        if (event->type() == SuccessEventType) {
            SuccessEvent *e = static_cast<SuccessEvent *>(event);
            d->send_frame(d->bufferResource, e->time, ScreencastPrivate::transform_normal);
        } else if (event->type() == FailedEventType) {
            FailedEvent *e = static_cast<FailedEvent *>(event);
            d->send_failed(static_cast<int32_t>(e->error), d->bufferResource);
        } else {
            return QObject::event(event);
        }
    }

    d->bufferResource = Q_NULLPTR;
    d->buffer = Q_NULLPTR;

    wl_client_flush(d->resource()->client());

    return true;
}

} // namespace Server

} // namespace GreenIsland

#include "moc_screencaster.cpp"
