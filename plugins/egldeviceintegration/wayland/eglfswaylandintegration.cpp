/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QThread>
#include <QtGui/QSurface>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtPlatformHeaders/QEGLNativeContext>

#include <GreenIsland/Platform/EglFSIntegration>

#include "eglfswaylandcontext.h"
#include "eglfswaylandintegration.h"
#include "eglfswaylandwindow.h"
#include "eglfswaylandscreen.h"
#include "eglfswaylandlogging.h"

namespace GreenIsland {

namespace Platform {

EglFSWaylandIntegration::EglFSWaylandIntegration()
    : QObject()
    , m_thread(new QThread())
    , m_connection(new Client::ClientConnection())
    , m_registry(new Client::Registry(this))
    , m_compositor(Q_NULLPTR)
    , m_fullScreenShell(Q_NULLPTR)
    , m_seat(Q_NULLPTR)
    , m_input(Q_NULLPTR)
    , m_touchDevice(Q_NULLPTR)
{
    // Connect from a separate thread
    m_connection->moveToThread(m_thread);
    m_thread->start();
}

void EglFSWaylandIntegration::platformInit()
{
    // Make sure EGL is using the wayland platform
    qputenv("EGL_PLATFORM", QByteArrayLiteral("wayland"));

    EglFSIntegration *integration = static_cast<EglFSIntegration *>(
                QGuiApplicationPrivate::platformIntegration());
    Q_ASSERT(integration);

    // Bind
    connect(m_registry, &Client::Registry::compositorAnnounced, this, [this](quint32 name, quint32 version) {
        m_compositor = m_registry->createCompositor(name, version);
    });
    connect(m_registry, &Client::Registry::fullscreenShellAnnounced, this, [this](quint32 name, quint32 version) {
        m_fullScreenShell = m_registry->createFullScreenShell(name, version);
    });
    connect(m_registry, &Client::Registry::outputAnnounced, this, [this](quint32 name, quint32 version) {
        Client::Output *output = m_registry->createOutput(name, version, this);
        m_outputs.append(output);
    });
    connect(m_registry, &Client::Registry::seatAnnounced, this, [this](quint32 name, quint32 version) {
        m_seat = m_registry->createSeat(name, version, this);

        delete m_input;
        m_input = new EglFSWaylandInput(m_seat, this);

        connect(m_seat, &Client::Seat::keyboardAdded,
                this, &EglFSWaylandIntegration::keyboardAdded);
        connect(m_seat, &Client::Seat::pointerAdded,
                this, &EglFSWaylandIntegration::pointerAdded);
        connect(m_seat, &Client::Seat::touchAdded,
                this, &EglFSWaylandIntegration::touchAdded);
        connect(m_seat, &Client::Seat::touchRemoved,
                this, &EglFSWaylandIntegration::touchRemoved);
    });

    // Connect to the compositor
    m_connection->synchronousConnection();
    if (!m_connection->isConnected()) {
        qCWarning(gLcEglFSWayland) << "Connection to" << m_connection->socketName() << "failed";
        return;
    }

    // Setup registry
    m_registry->create(m_connection->display());
    m_registry->setup();

    // Wait until all interfaces are bound
    while (!m_compositor || !m_seat || !m_fullScreenShell)
        m_connection->forceRoundTrip();

    qCDebug(gLcEglFSWayland) << "Platform initialized";
}

void EglFSWaylandIntegration::platformDestroy()
{
    if (m_fullScreenShell) {
        m_fullScreenShell->deleteLater();
        m_fullScreenShell = Q_NULLPTR;
    }

    if (m_compositor) {
        m_compositor->deleteLater();
        m_compositor = Q_NULLPTR;
    }

    m_registry->deleteLater();
    m_registry = Q_NULLPTR;

    m_connection->deleteLater();
    m_connection = Q_NULLPTR;

    m_thread->quit();
    m_thread->wait();
}

EGLNativeDisplayType EglFSWaylandIntegration::platformDisplay() const
{
    return m_connection->display();
}

bool EglFSWaylandIntegration::handlesInput()
{
    return true;
}

bool EglFSWaylandIntegration::usesVtHandler()
{
    return false;
}

bool EglFSWaylandIntegration::usesDefaultScreen()
{
    return false;
}

void EglFSWaylandIntegration::screenInit()
{
    m_connection->flushRequests();

    while (true) {
        bool ready = !m_outputs.isEmpty();

        for (int i = 0; ready && i < m_outputs.count(); ++i) {
            if (m_outputs.at(i)->geometry() == QRect(0, 0, 0, 0))
                ready = false;
        }

        if (!ready)
            m_connection->blockingReadEvents();
        else
            break;
    }

    EglFSIntegration *integration = static_cast<EglFSIntegration *>(
                QGuiApplicationPrivate::platformIntegration());
    QList<QPlatformScreen *> siblings;
    Q_FOREACH (Client::Output *output, m_outputs) {
        EglFSWaylandScreen *screen = new EglFSWaylandScreen(integration->display(), this, output);
        siblings.append(screen);
        integration->addScreen(screen);
    }

    Q_FOREACH (QPlatformScreen *screen, siblings)
        static_cast<EglFSWaylandScreen *>(screen)->setVirtualSiblings(siblings);

    qCDebug(gLcEglFSWayland) << "Screens initialized";
}

QPlatformWindow *EglFSWaylandIntegration::createPlatformWindow(QWindow *window)
{
    if (window->surfaceType() != QSurface::OpenGLSurface)
        qFatal("The wayland EGL device integration only support OpenGL surfaces");

    EglFSWaylandWindow *w = new EglFSWaylandWindow(this, window);
    w->create();
    w->requestActivateWindow();
    return w;
}

QPlatformOpenGLContext *EglFSWaylandIntegration::createPlatformOpenGLContext(QOpenGLContext *context)
{
    EglFSIntegration *integration = static_cast<EglFSIntegration *>(QGuiApplicationPrivate::platformIntegration());
    EGLDisplay dpy = context->screen() ? static_cast<EglFSWaylandScreen *>(context->screen()->handle())->display() : integration->display();
    QPlatformOpenGLContext *share = context->shareHandle();
    QVariant nativeHandle = context->nativeHandle();

    EglFSWaylandContext *ctx;
    QSurfaceFormat adjustedFormat = surfaceFormatFor(context->format());
    if (nativeHandle.isNull()) {
        EGLConfig config = EglFSIntegration::chooseConfig(dpy, adjustedFormat);
        ctx = new EglFSWaylandContext(adjustedFormat, share, dpy, &config, QVariant());
    } else {
        ctx = new EglFSWaylandContext(adjustedFormat, share, dpy, 0, nativeHandle);
    }
    nativeHandle = QVariant::fromValue<QEGLNativeContext>(QEGLNativeContext(ctx->eglContext(), dpy));

    context->setNativeHandle(nativeHandle);
    return ctx;
}

bool EglFSWaylandIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case QPlatformIntegration::ThreadedPixmaps:
    case QPlatformIntegration::OpenGL:
    case QPlatformIntegration::ThreadedOpenGL:
    case QPlatformIntegration::MultipleWindows:
        return true;
    case QPlatformIntegration::ForeignWindows:
    case QPlatformIntegration::NonFullScreenWindows:
    case QPlatformIntegration::WindowManagement:
        return false;
    default:
        break;
    }

    return false;
}

void EglFSWaylandIntegration::keyboardAdded()
{
    connect(m_seat->keyboard(), &Client::Keyboard::keymapChanged,
            m_input, &EglFSWaylandInput::keymapChanged);
    connect(m_seat->keyboard(), &Client::Keyboard::keyPressed,
            m_input, &EglFSWaylandInput::keyPressed);
    connect(m_seat->keyboard(), &Client::Keyboard::keyReleased,
            m_input, &EglFSWaylandInput::keyReleased);
    connect(m_seat->keyboard(), &Client::Keyboard::modifiersChanged,
            m_input, &EglFSWaylandInput::keyboardModifiersChanged);
}

void EglFSWaylandIntegration::pointerAdded()
{
    connect(m_seat->pointer(), &Client::Pointer::enter,
            m_input, &EglFSWaylandInput::pointerEnter);
    connect(m_seat->pointer(), &Client::Pointer::leave,
            m_input, &EglFSWaylandInput::pointerLeave);
    connect(m_seat->pointer(), &Client::Pointer::motion,
            m_input, &EglFSWaylandInput::pointerMotion);
    connect(m_seat->pointer(), &Client::Pointer::buttonPressed,
            m_input, &EglFSWaylandInput::pointerButtonPressed);
    connect(m_seat->pointer(), &Client::Pointer::buttonReleased,
            m_input, &EglFSWaylandInput::pointerButtonReleased);
    connect(m_seat->pointer(), &Client::Pointer::axisChanged,
            m_input, &EglFSWaylandInput::pointerAxisChanged);
}

void EglFSWaylandIntegration::touchAdded()
{
    m_touchDevice = new QTouchDevice();
    m_touchDevice->setName(m_seat->name());
    m_touchDevice->setType(QTouchDevice::TouchPad);
    QWindowSystemInterface::registerTouchDevice(m_touchDevice);
    m_input->setTouchDevice(m_touchDevice);

    connect(m_seat->touch(), &Client::Touch::sequenceStarted,
            m_input, &EglFSWaylandInput::touchSequenceStarted);
    connect(m_seat->touch(), &Client::Touch::sequenceFinished,
            m_input, &EglFSWaylandInput::touchSequenceFinished);
    connect(m_seat->touch(), &Client::Touch::sequenceCanceled,
            m_input, &EglFSWaylandInput::touchSequenceCanceled);

    connect(m_seat->touch(), &Client::Touch::pointAdded,
            m_input, &EglFSWaylandInput::touchPointAdded);
    connect(m_seat->touch(), &Client::Touch::pointRemoved,
            m_input, &EglFSWaylandInput::touchPointRemoved);
    connect(m_seat->touch(), &Client::Touch::pointMoved,
            m_input, &EglFSWaylandInput::touchPointMoved);
}

void EglFSWaylandIntegration::touchRemoved()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QWindowSystemInterface::unregisterTouchDevice(m_touchDevice);
#endif
    delete m_touchDevice;
    m_touchDevice = Q_NULLPTR;
    m_input->setTouchDevice(m_touchDevice);
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_eglfswaylandintegration.cpp"
