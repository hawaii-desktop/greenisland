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

#include <QtCore/QCoreApplication>
#include <QtCore/QMutexLocker>
#include <QtOpenGL/QGLFunctions>
#include "quickoutput.h"

#include "greenislandrecorder.h"

#include <sys/time.h>
#include <grp.h>

Q_LOGGING_CATEGORY(RECORDER_PROTOCOL, "greenisland.protocols.greenisland.recorder")

namespace GreenIsland {

static uint32_t getTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static const QEvent::Type FrameEventType = (QEvent::Type)QEvent::registerEventType();
static const QEvent::Type FailedEventType = (QEvent::Type)QEvent::registerEventType();

class FrameEvent : public QEvent
{
public:
    FrameEvent(uint32_t t)
        : QEvent(FrameEventType)
        , time(t)
    { }
    uint32_t time;
};

class FailedEvent : public QEvent
{
public:
    FailedEvent(int r)
        : QEvent(FailedEventType)
        , result(r)
    { }
    int result;
};

/*
 * GreenIslandRecorderManager
 */

GreenIslandRecorderManager::GreenIslandRecorderManager()
    : GlobalInterface()
{
}

const wl_interface* GreenIslandRecorderManager::interface() const
{
    return &greenisland_recorder_manager_interface;
}

void GreenIslandRecorderManager::recordFrame(QWindow *window)
{
    QMutexLocker lock(&m_mutex);
    if (m_requests.isEmpty())
        return;

    uchar *pixels;
    uint32_t time = getTime();
    Q_FOREACH (GreenIslandRecorder *recorder, m_requests.values(window)) {
        wl_shm_buffer *buffer = recorder->buffer();
        pixels = static_cast<uchar *>(wl_shm_buffer_get_data(buffer));
        int width = wl_shm_buffer_get_width(buffer);
        int height = wl_shm_buffer_get_height(buffer);
        int stride = wl_shm_buffer_get_stride(buffer);
        int bpp = 4;

        if (width < window->width() || height < window->height() || stride < window->width() * bpp) {
            qApp->postEvent(recorder, new FailedEvent(QtWaylandServer::greenisland_recorder::result_bad_buffer));
            continue;
        }

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        qApp->postEvent(recorder, new FrameEvent(time));

        m_requests.remove(window, recorder);
    }
}

void GreenIslandRecorderManager::requestFrame(QWindow *window, GreenIslandRecorder *recorder)
{
    QMutexLocker lock(&m_mutex);
    m_requests.insert(window, recorder);
}

void GreenIslandRecorderManager::remove(QWindow *window, GreenIslandRecorder *recorder)
{
    QMutexLocker lock(&m_mutex);
    m_requests.remove(window, recorder);
}

void GreenIslandRecorderManager::bind(wl_client *client, quint32 version, quint32 id)
{
    Q_UNUSED(version)

#ifdef SCREENCASTER_SECURITY_CHECK
    Resource *res = add(client, id, version);

    gid_t gid;
    wl_client_get_credentials(client, Q_NULLPTR, Q_NULLPTR, &gid);
    group *g = getgrgid(gid);
    if (strcmp(g->gr_name, "privileged") != 0) {
        wl_resource_post_error(res->handle,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "Permission to bind greenisland_recorder_manager denied");
        wl_resource_destroy(res->handle);
    }
#else
    add(client, id, version);
#endif
}

void GreenIslandRecorderManager::recorder_manager_create_recorder(Resource *resource, uint32_t id, ::wl_resource *outputResource)
{
    QuickOutput *output = static_cast<QuickOutput *>(AbstractOutput::fromResource(outputResource));
    if (!output) {
        qCWarning(RECORDER_PROTOCOL) << "Couldn't find output from resource";
        return;
    }
    new GreenIslandRecorder(this, resource->client(), id, output->quickWindow());
}

/*
 * GreenIslandRecorder
 */

GreenIslandRecorder::GreenIslandRecorder(GreenIslandRecorderManager *manager, wl_client *client, quint32 id, QQuickWindow *window)
    : QtWaylandServer::greenisland_recorder(client, id, 1)
    , m_manager(manager)
    , m_bufferResource(Q_NULLPTR)
    , m_client(client)
    , m_window(window)
{
    send_setup(window->width(), window->height(), window->width() * 4, WL_SHM_FORMAT_RGBA8888);
}

GreenIslandRecorder::~GreenIslandRecorder()
{
    m_manager->remove(m_window, this);
}

void GreenIslandRecorder::recorder_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void GreenIslandRecorder::recorder_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void GreenIslandRecorder::recorder_record_frame(Resource *resource, ::wl_resource *buffer)
{
    Q_UNUSED(resource);

    if (m_bufferResource)
        send_cancelled(buffer);

    m_bufferResource = buffer;
    m_buffer = wl_shm_buffer_get(buffer);
    if (m_buffer) {
        m_manager->requestFrame(m_window, this);
    } else {
        m_bufferResource = Q_NULLPTR;
        send_failed(result_bad_buffer, buffer);
    }
}

void GreenIslandRecorder::recorder_repaint(Resource *resource)
{
    Q_UNUSED(resource);

    if (m_bufferResource)
        m_window->update();
}

bool GreenIslandRecorder::event(QEvent *e)
{
    if (e->type() == FrameEventType) {
        FrameEvent *fe = static_cast<FrameEvent *>(e);
        send_frame(m_bufferResource, fe->time, QtWaylandServer::greenisland_recorder::transform_y_inverted);
    } else if (e->type() == FailedEventType) {
        FailedEvent *fe = static_cast<FailedEvent *>(e);
        send_failed(fe->result, m_bufferResource);
    } else {
        return QObject::event(e);
    }

    m_bufferResource = Q_NULLPTR;
    wl_client_flush(client());
    return true;
}

}
