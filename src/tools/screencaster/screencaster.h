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

#ifndef SCREENCASTER_H
#define SCREENCASTER_H

#include <QtCore/QObject>
#include <QtCore/QMutex>

#include <wayland-client.h>

class QScreen;

struct wl_display;
struct wl_registry;
struct greenisland_recorder_manager;
struct greenisland_recorder;

class Buffer;
class BuffersHandler;

class ScreenCaster : public QObject
{
    Q_OBJECT
public:
    ScreenCaster(QIODevice *output, unsigned int numberOfFrames = 0);
    ~ScreenCaster();

private slots:
    void start();
    void recordFrame();

private:
    static void global(void *data, wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
    static void globalRemove(void *data, wl_registry *registry, uint32_t id);
    static void setup(void *data, greenisland_recorder *recorder, int width, int height, int stride, int format);
    static void frame(void *data, greenisland_recorder *recorder, wl_buffer *buffer, uint32_t time, int transform);
    static void failed(void *data, greenisland_recorder *recorder, int result, wl_buffer *buffer);
    static void cancel(void *data, greenisland_recorder *recorder, wl_buffer *buffer);

    wl_display *m_display;
    wl_registry *m_registry;
    wl_shm *m_shm;
    greenisland_recorder_manager *m_manager;
    greenisland_recorder *m_recorder;
    QScreen *m_screen;
    QList<Buffer *> m_buffers;
    bool m_starving;
    QThread *m_buffersThread;
    BuffersHandler *m_buffersHandler;
    QMutex m_mutex;

    friend class BuffersHandler;
};

#endif // SCREENCASTER_H
