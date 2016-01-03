/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>

#include <GreenIsland/Client/Output>

#include "application.h"
#include "recorder.h"

static const QEvent::Type StartupEventType =
        static_cast<QEvent::Type>(QEvent::registerEventType());

StartupEvent::StartupEvent()
    : QEvent(StartupEventType)
{
}

Application::Application(const QString &fileName, int maxFrames,
                         bool stillImages, QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_maxFrames(maxFrames)
    , m_curFrame(0)
    , m_thread(new QThread())
    , m_connection(Client::ClientConnection::fromQt())
    , m_registry(new Client::Registry(this))
    , m_shm(Q_NULLPTR)
    , m_screencaster(Q_NULLPTR)
{
    // Wayland connection in a separate thread
    Q_ASSERT(m_connection);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    // Create the recorder unless we want to save still imagess
    if (stillImages) {
        m_recorder = Q_NULLPTR;
    } else {
        m_recorder = new Recorder(fileName);
        if (!m_recorder->open())
            qFatal("Failed to open output file for writing");
    }
}

Application::~Application()
{
    delete m_screencaster;
    delete m_shm;
    delete m_registry;
    delete m_connection;

    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }

    delete m_recorder;
}

bool Application::event(QEvent *event)
{
    if (event->type() == StartupEventType) {
        initialize();
        return true;
    }

    return QObject::event(event);
}

void Application::initialize()
{
    if (m_initialized)
        return;

    // Interfaces
    connect(m_registry, &Client::Registry::interfacesAnnounced,
            this, &Application::interfacesAnnounced);
    connect(m_registry, &Client::Registry::interfaceAnnounced,
            this, &Application::interfaceAnnounced);

    // Setup registry
    m_registry->create(m_connection->display());
    m_registry->setup();

    m_initialized = true;
}

void Application::interfacesAnnounced()
{
    if (!m_shm)
        qCritical("Unable to create shared memory buffers");

    if (!m_screencaster)
        qCritical("Wayland compositor doesn't have screencaster capabilities");

    Client::Output *output =
            Client::Output::fromQt(QGuiApplication::primaryScreen());
    Client::Screencast *screencast = m_screencaster->capture(output);

    connect(screencast, &Client::Screencast::setupFailed, this, [this] {
        qCritical("Failed to setup buffer for recording");
        QGuiApplication::quit();
    });
    connect(screencast, &Client::Screencast::frameRecorded, this,
            [this](Client::Buffer *buffer, quint32 time, Client::Screencast::Transform transform) {
        // Increment frame counter
        m_curFrame++;

        // Grab an image of the frame
        QImage image = buffer->image();
        if (transform == Client::Screencast::TransformYInverted)
            image = image.mirrored(false, true);

        // Write the image
        if (m_recorder)
            m_recorder->write(time, buffer->size(), buffer->stride(), image);
        else
            image.save(QString().sprintf("frame%d.png", m_curFrame));

        // Limit frames
        if (m_maxFrames > 0 && m_curFrame >= m_maxFrames)
            QGuiApplication::quit();
    });
    connect(screencast, &Client::Screencast::failed, this,
            [this](Client::Screencast::RecordError error) {
        switch (error) {
        case Client::Screencast::ErrorBadBuffer:
            qCritical("Failed to record a frame: bad buffer");
            QGuiApplication::quit();
            break;
        }
    });
    connect(screencast, &Client::Screencast::canceled, this, [this] {
        qWarning("Frame recording canceled");
    });
}

void Application::interfaceAnnounced(const QByteArray &interface,
                                     quint32 name, quint32 version)
{
    if (interface == Client::Shm::interfaceName())
        m_shm = m_registry->createShm(name, version);
    else if (interface == Client::Screencaster::interfaceName())
        m_screencaster = m_registry->createScreencaster(m_shm, name, version);
}

#include "moc_application.cpp"
