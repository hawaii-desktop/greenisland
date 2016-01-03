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

#include "buffer.h"
#include "buffer_p.h"
#include "output_p.h"
#include "registry.h"
#include "screencaster.h"
#include "screencaster_p.h"
#include "shm.h"
#include "shmpool.h"
#include "shmpool_p.h"

namespace GreenIsland {

namespace Client {

class ScreencastBuffer
{
public:
    ScreencastBuffer()
        : busy(false)
    {}

    BufferPtr buffer;
    bool busy;
};

static const QEvent::Type FrameEventType =
        static_cast<QEvent::Type>(QEvent::registerEventType());

class FrameEvent : public QEvent
{
public:
    FrameEvent(ScreencastBuffer *_buffer, quint32 _time, qint32 _transform)
        : QEvent(FrameEventType)
        , buffer(_buffer)
        , time(_time)
        , transform(static_cast<Screencast::Transform>(_transform))
    {}

    ScreencastBuffer *buffer;
    quint32 time;
    Screencast::Transform transform;
};

class ScreencastHandler : public QObject
{
public:
    ScreencastHandler(QObject *parent = Q_NULLPTR)
        : QObject(parent)
        , screencast(Q_NULLPTR)
    {}

    Screencast *screencast;

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE
    {
        if (event->type() == FrameEventType) {
            FrameEvent *e = static_cast<FrameEvent *>(event);

            // We are not using this buffer anymore
            e->buffer->busy = false;

            // Record another frame
            Q_ASSERT(screencast);
            if (ScreencastPrivate::get(screencast)->starving)
                ScreencastPrivate::get(screencast)->recordFrame();

            Q_EMIT screencast->frameRecorded(e->buffer->buffer.data(), e->time, e->transform);

            return true;
        }

        return QObject::event(event);
    }
};

/*
 * ScreencasterPrivate
 */

ScreencasterPrivate::ScreencasterPrivate()
    : QtWayland::greenisland_screencaster()
    , registry(Q_NULLPTR)
    , shm(Q_NULLPTR)
{
}

ScreencasterPrivate::~ScreencasterPrivate()
{
    while (!requests.isEmpty())
        delete requests.takeLast();
}

/*
 * Screencaster
 */

Screencaster::Screencaster(Shm *shm, QObject *parent)
    : QObject(*new ScreencasterPrivate(), parent)
{
    qRegisterMetaType<Screencast::Transform>("Transform");
    qRegisterMetaType<Screencast::RecordError>("RecordError");

    d_func()->shm = shm;
}

Screencast *Screencaster::capture(Output *output)
{
    Q_D(Screencaster);

    wl_output *o = OutputPrivate::get(output)->object();

    Screencast *screencast = new Screencast(this);
    ScreencastPrivate *dScreencast = ScreencastPrivate::get(screencast);
    dScreencast->init(d->capture(o));
    if (!dScreencast->isInitialized()) {
        delete screencast;
        screencast = Q_NULLPTR;
    }
    return screencast;
}

QByteArray Screencaster::interfaceName()
{
    return QByteArrayLiteral("greenisland_screencaster");
}

/*
 * ScreencastPrivate
 */

ScreencastPrivate::ScreencastPrivate(Screencaster *s)
    : QtWayland::greenisland_screencast()
    , shmPool(Q_NULLPTR)
    , screencaster(s)
    , starving(false)
    , thread(new QThread())
    , handler(new ScreencastHandler())
{
}

ScreencastPrivate::~ScreencastPrivate()
{
    thread->quit();
    thread->wait();

    delete handler;
    delete thread;

    destroy();
    wl_display_flush(ScreencasterPrivate::get(screencaster)->registry->display());

    while (!buffers.isEmpty())
        delete buffers.takeLast();

    delete shmPool;
}

void ScreencastPrivate::recordFrame()
{
    // Pick up a free buffer
    ScreencastBuffer *buffer = Q_NULLPTR;
    Q_FOREACH (ScreencastBuffer *sb, buffers) {
        if (!sb->buffer.isNull() && !sb->busy) {
            buffer = sb;
            break;
        }
    }
    if (buffer) {
        record(BufferPrivate::get(buffer->buffer.data())->object());
        wl_display_flush(ScreencasterPrivate::get(screencaster)->registry->display());
        buffer->busy = true;
        starving = false;
    } else {
        qWarning("Frame recording is starving: no free buffer");
        starving = true;
    }
}

void ScreencastPrivate::screencast_setup(int32_t width, int32_t height,
                                         int32_t stride, int32_t f)
{
    Q_Q(Screencast);

    Shm::Format format = static_cast<Shm::Format>(f);
    ScreencasterPrivate *dScreencaster = ScreencasterPrivate::get(screencaster);

    // The compositor should give us a supported format (in theory)
    // but who knows, better check it
    if (!Q_UNLIKELY(dScreencaster->shm->formats().contains(format))) {
        qWarning("Unsupported format %s", qPrintable(Shm::formatToString(format)));
        Q_EMIT q->setupFailed();
        return;
    }

    qint32 size = stride * height; // equals to width * bpp * height

    // Create the shm pool
    if (!shmPool)
        shmPool = dScreencaster->shm->createPool(size);

    // Create several buffers that can be reused later
    for (int i = 0; i < 6; i++) {
        BufferPtr buffer = shmPool->createBuffer(QSize(width, height),
                                                 stride, Q_NULLPTR, format);
        ScreencastBuffer *screencastBuffer = new ScreencastBuffer();
        screencastBuffer->buffer = buffer;
        buffers.append(screencastBuffer);
    }

    // Ask the compositor to record a frame
    recordFrame();

    Q_EMIT q->setupDone(QSize(width, height), stride);
}

void ScreencastPrivate::screencast_frame(struct ::wl_buffer *buffer,
                                         uint32_t time, int32_t transform)
{
    Q_Q(Screencast);

    // Serialize frame recording
    QMutexLocker locker(&recordMutex);
    recordFrame();

    Buffer *b = BufferPrivate::fromWlBuffer(buffer);
    Q_FOREACH (ScreencastBuffer *sb, buffers) {
        if (!sb->buffer.isNull() && sb->buffer.data() == b) {
            QCoreApplication::postEvent(handler, new FrameEvent(sb, time, static_cast<Screencast::Transform>(transform)));
            break;
        }
    }
}

void ScreencastPrivate::screencast_failed(int32_t error,
                                          struct ::wl_buffer *buffer)
{
    Q_UNUSED(buffer);

    Q_Q(Screencast);
    Q_EMIT q->failed(static_cast<Screencast::RecordError>(error));
}

void ScreencastPrivate::screencast_cancelled(struct ::wl_buffer *buffer)
{
    Q_Q(Screencast);

    Buffer *b = BufferPrivate::fromWlBuffer(buffer);
    Q_FOREACH (ScreencastBuffer *sb, buffers) {
        if (sb->buffer.isNull() && sb->buffer.data() == b) {
            sb->busy = false;
            break;
        }
    }

    Q_EMIT q->canceled();
}

/*
 * Screencast
 */

Screencast::Screencast(QObject *parent)
    : QObject(*new ScreencastPrivate(static_cast<Screencaster *>(parent)), parent)
{
    d_func()->handler->screencast = this;
    d_func()->handler->moveToThread(d_func()->thread);
    d_func()->thread->start();
}

QByteArray Screencast::interfaceName()
{
    return QByteArrayLiteral("greenisland_screencast");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_screencaster.cpp"
