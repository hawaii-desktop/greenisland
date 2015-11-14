/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Original Author(s):
 *    Giulio Camuffo <giulio.camuffo@jollamobile.com>
 *
 * $BEGIN_LICENSE:LGPL2.1$
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging
 * of this file.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLANDRECORDER_H
#define GREENISLANDRECORDER_H

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMultiHash>
#include <QtCore/QMutex>
#include <GreenIsland/Compositor/QWaylandGlobalInterface>

#include "qwayland-server-greenisland-recorder.h"

struct wl_shm_buffer;
struct wl_client;

class QWindow;
class QQuickWindow;
class QEvent;

Q_DECLARE_LOGGING_CATEGORY(RECORDER_PROTOCOL)

namespace GreenIsland {

class GreenIslandRecorder;

class GreenIslandRecorderManager : public QWaylandGlobalInterface, public QtWaylandServer::greenisland_recorder_manager
{
public:
    GreenIslandRecorderManager();

    const wl_interface* interface() const Q_DECL_OVERRIDE;

    void recordFrame(QWindow *window);
    void requestFrame(QWindow *window, GreenIslandRecorder *recorder);
    void remove(QWindow *window, GreenIslandRecorder *recorder);

protected:
    void bind(wl_client *client, quint32 version, quint32 id) Q_DECL_OVERRIDE;
    void recorder_manager_create_recorder(Resource *resource,
                                          uint32_t id,
                                          ::wl_resource *outputResource) Q_DECL_OVERRIDE;

private:
    QMultiHash<QWindow *, GreenIslandRecorder *> m_requests;
    QMutex m_mutex;
};

class GreenIslandRecorder : public QObject, public QtWaylandServer::greenisland_recorder
{
public:
    GreenIslandRecorder(GreenIslandRecorderManager *manager, wl_client *client, quint32 id, QQuickWindow *window);
    ~GreenIslandRecorder();

    wl_shm_buffer *buffer() const { return m_buffer; }
    wl_client *client() const { return m_client; }

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    void recorder_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void recorder_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void recorder_record_frame(Resource *resource, ::wl_resource *buffer) Q_DECL_OVERRIDE;
    void recorder_repaint(Resource *resource) Q_DECL_OVERRIDE;

private:
    GreenIslandRecorderManager *m_manager;
    wl_resource *m_bufferResource;
    wl_shm_buffer *m_buffer;
    wl_client *m_client;
    QQuickWindow *m_window;
};

}

#endif // GREENISLANDRECORDER_H
