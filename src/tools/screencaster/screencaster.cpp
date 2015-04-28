/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * Original Author(s):
 *    Giulio Camuffo <giulio.camuffo@jollamobile.com>
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

#include <QtCore/QDebug>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QImage>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "wayland-greenisland-recorder-client-protocol.h"
#include "screencaster.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define WCAP_HEADER_MAGIC 0x57434150

#define WCAP_FORMAT_XRGB8888 0x34325258
#define WCAP_FORMAT_XBGR8888 0x34324258
#define WCAP_FORMAT_RGBX8888 0x34325852
#define WCAP_FORMAT_BGRX8888 0x34325842

class Buffer
{
public:
    static Buffer *create(wl_shm *shm, int width, int height, int stride, int format)
    {
        int size = stride * height;

        char filename[] = "/tmp/greenisland-recorder-shm-XXXXXX";
        int fd = mkstemp(filename);
        if (fd < 0) {
            qWarning("creating a buffer file for %d B failed: %m\n", size);
            return Q_NULLPTR;
        }
        int flags = fcntl(fd, F_GETFD);
        if (flags != -1)
            fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

        if (ftruncate(fd, size) < 0) {
            qWarning("ftruncate failed: %s", strerror(errno));
            close(fd);
            return Q_NULLPTR;
        }

        uchar *data = (uchar *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        unlink(filename);
        if (data == (uchar *)MAP_FAILED) {
            qWarning("mmap failed: %m\n");
            close(fd);
            return Q_NULLPTR;
        }

        Buffer *buf = new Buffer;

        wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
        buf->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
        wl_buffer_set_user_data(buf->buffer, buf);
        wl_shm_pool_destroy(pool);
        buf->data = data;
        buf->image = QImage(data, width, height, stride, QImage::Format_RGBA8888);
        buf->busy = false;
        close(fd);
        return buf;
    }

    wl_buffer *buffer;
    uchar *data;
    QImage image;
    bool busy;
};

static const QEvent::Type FrameEventType = (QEvent::Type)QEvent::registerEventType();

class FrameEvent : public QEvent
{
public:
    FrameEvent(Buffer *b, uint32_t time, int tr)
        : QEvent(FrameEventType)
        , buffer(b)
        , timestamp(time)
        , transform(tr)
    {
    }

    Buffer *buffer;
    uint32_t timestamp;
    int transform;
};

class BuffersHandler : public QObject
{
public:
    BuffersHandler(QIODevice *output, unsigned int numberOfFrames)
        : output(output)
        , countingFrames(numberOfFrames > 0)
        , remainingFrames(numberOfFrames)
    {
        stream.setDevice(output);
    }

    ~BuffersHandler()
    {
        output->close();
    }

    void initialize(quint32 width, quint32 height, quint32 format)
    {
        // Write WCAP header
        qDebug() << ":::::::::::::";
        stream << WCAP_HEADER_MAGIC << format << width << height;
    }

    bool event(QEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type() == FrameEventType) {
            FrameEvent *fe = static_cast<FrameEvent *>(e);
            Buffer *buf = fe->buffer;
            static int id = 0;
            QImage img = fe->transform == GREENISLAND_RECORDER_TRANSFORM_Y_INVERTED ? buf->image.mirrored(false, true) : buf->image;
            buf->busy = false;
            if (rec->m_starving)
                rec->recordFrame();

            //output->write((const char*) img.bits(), img.byteCount());
            // Write frame header
            stream << fe->timestamp << 1;

            // Write rectangle
            stream << 0 << 0 << fe->buffer->image.width() << fe->buffer->image.height();

            // Write data
            stream << img.bits();

            if (countingFrames && --remainingFrames == 0)
                QCoreApplication::quit();

            return true;
        }
        return QObject::event(e);
    }

    ScreenCaster *rec;

private:
    QIODevice *output;
    bool countingFrames;
    unsigned int remainingFrames;
    QDataStream stream;
};

static void callback(void *data, wl_callback *cb, uint32_t time)
{
    Q_UNUSED(time)
    wl_callback_destroy(cb);
    QMetaObject::invokeMethod(static_cast<ScreenCaster *>(data), "start");
}

ScreenCaster::ScreenCaster(QIODevice *output, unsigned int numberOfFrames)
    : QObject()
    , m_manager(Q_NULLPTR)
    , m_starving(false)
{
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    m_display = static_cast<wl_display *>(native->nativeResourceForIntegration("display"));
    m_registry = wl_display_get_registry(m_display);

    static const wl_registry_listener registryListener = {
        global,
        globalRemove
    };
    wl_registry_add_listener(m_registry, &registryListener, this);

    wl_callback *cb = wl_display_sync(m_display);
    static const wl_callback_listener callbackListener = {
        callback
    };
    wl_callback_add_listener(cb, &callbackListener, this);

    m_buffersThread = new QThread;
    m_buffersHandler = new BuffersHandler(output, numberOfFrames);
    m_buffersHandler->rec = this;
    m_buffersHandler->moveToThread(m_buffersThread);
    m_buffersThread->start();
}

ScreenCaster::~ScreenCaster()
{
    m_buffersThread->quit();
    m_buffersThread->wait();
    greenisland_recorder_destroy(m_recorder);
    delete m_buffersHandler;
    delete m_buffersThread;
}

void ScreenCaster::start()
{
    if (!m_manager)
        qFatal("The greenisland_recorder_manager global is not available.");

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    m_screen = QGuiApplication::screens().first();
    wl_output *output = static_cast<wl_output *>(native->nativeResourceForScreen("output", m_screen));

    m_recorder = greenisland_recorder_manager_create_recorder(m_manager, output);
    static const greenisland_recorder_listener recorderListener = {
        setup,
        frame,
        failed,
        cancel
    };
    greenisland_recorder_add_listener(m_recorder, &recorderListener, this);
}

void ScreenCaster::recordFrame()
{
    Buffer *buf = Q_NULLPTR;
    foreach (Buffer *b, m_buffers) {
        if (!b->busy) {
            buf = b;
            break;
        }
    }
    if (buf) {
        greenisland_recorder_record_frame(m_recorder, buf->buffer);
        wl_display_flush(m_display);
        buf->busy = true;
        m_starving = false;
    } else {
        qWarning("No free buffers.");
        m_starving = true;
    }
}

QString formatToString(int format)
{
    switch (format) {
        case WL_SHM_FORMAT_RGBA8888:
            return QString("rgba8888");
        default:
            break;
    }

    return QString("unknown");
}

void ScreenCaster::setup(void *data, greenisland_recorder *recorder, int width, int height, int stride, int format)
{
    ScreenCaster *rec = static_cast<ScreenCaster *>(data);
    QMutexLocker lock(&rec->m_mutex);

    qWarning("Recording with output size %dx%d format %s",
             width, height, formatToString(format).toUtf8().constData());

    rec->m_buffersHandler->initialize(width, height, WCAP_FORMAT_RGBX8888);

    for (int i = 0; i < 6; ++i) {
        Buffer *buffer = Buffer::create(rec->m_shm, width, height, stride, format);
        if (!buffer)
            qFatal("Failed to create a buffer.");
        rec->m_buffers << buffer;
    }
    rec->recordFrame();
}

void ScreenCaster::frame(void *data, greenisland_recorder *recorder, wl_buffer *buffer, uint32_t timestamp, int transform)
{
    Q_UNUSED(recorder)

    ScreenCaster *rec = static_cast<ScreenCaster *>(data);
    static uint32_t time = 0;

    QMutexLocker lock(&rec->m_mutex);
    rec->recordFrame();
    Buffer *buf = static_cast<Buffer *>(wl_buffer_get_user_data(buffer));

    time = timestamp;

    qApp->postEvent(rec->m_buffersHandler, new FrameEvent(buf, timestamp, transform));
}

void ScreenCaster::failed(void *data, greenisland_recorder *recorder, int result, wl_buffer *buffer)
{
    Q_UNUSED(data)
    Q_UNUSED(recorder)
    Q_UNUSED(buffer)

    qFatal("Failed to record a frame, result %d.", result);
}

void ScreenCaster::cancel(void *data, greenisland_recorder *recorder, wl_buffer *buffer)
{
    Q_UNUSED(recorder)

    ScreenCaster *rec = static_cast<ScreenCaster *>(data);

    QMutexLocker lock(&rec->m_mutex);
    Buffer *buf = static_cast<Buffer *>(wl_buffer_get_user_data(buffer));
    buf->busy = false;
}

void ScreenCaster::global(void *data, wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
    Q_UNUSED(registry)

    ScreenCaster *rec = static_cast<ScreenCaster *>(data);
    if (strcmp(interface, "greenisland_recorder_manager") == 0) {
        rec->m_manager = static_cast<greenisland_recorder_manager *>(wl_registry_bind(registry, id, &greenisland_recorder_manager_interface, qMin(version, 1u)));
    } else if (strcmp(interface, "wl_shm") == 0) {
        rec->m_shm = static_cast<wl_shm *>(wl_registry_bind(registry, id, &wl_shm_interface, qMin(version, 1u)));
    }
}

void ScreenCaster::globalRemove(void *data, wl_registry *registry, uint32_t id)
{
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(id)
}

#include "moc_screencaster.cpp"
