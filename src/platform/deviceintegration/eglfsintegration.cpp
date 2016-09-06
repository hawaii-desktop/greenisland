/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#include <QtGui/QSurfaceFormat>

#include <QtGui/qpa/qplatforminputcontextfactory_p.h>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixservices_p.h>

#include <QtPlatformHeaders/QEGLNativeContext>

#include "eglconvenience/eglconvenience.h"
#include "eglconvenience/eglplatformcontext.h"
#include "eglconvenience/eglpbuffer.h"
#include "deviceintegration/egldeviceintegration.h"
#include "deviceintegration/eglfscontext.h"
#include "deviceintegration/eglfscursor.h"
#include "deviceintegration/eglfsintegration.h"
#include "deviceintegration/eglfsnativeinterface.h"
#include "deviceintegration/eglfsoffscreenwindow.h"
#include "deviceintegration/eglfswindow.h"
#include "logind/logind.h"
#include "logging.h"
#include "libinput/libinputmanager_p.h"
#include "libinput/libinputhandler.h"
#include "platformcompositor/openglcompositorbackingstore.h"

static void initResources()
{
#ifndef QT_NO_CURSOR
    Q_INIT_RESOURCE(cursor);
#endif
}

namespace GreenIsland {

namespace Platform {

EglFSIntegration::EglFSIntegration()
    : QObject()
    , m_display(EGL_NO_DISPLAY)
    , m_inputContext(Q_NULLPTR)
    , m_fontDatabase(new QGenericUnixFontDatabase)
    , m_services(new QGenericUnixServices)
    , m_liHandler(Q_NULLPTR)
{
    initResources();

    m_nativeInterface.reset(new EglFSNativeInterface(this));
}

EGLDisplay EglFSIntegration::display() const
{
    return m_display;
}

EGLNativeDisplayType EglFSIntegration::nativeDisplay() const
{
    return egl_device_integration()->platformDisplay();
}

QPlatformInputContext *EglFSIntegration::inputContext() const
{
    return m_inputContext;
}

QPlatformFontDatabase *EglFSIntegration::fontDatabase() const
{
    return m_fontDatabase.data();
}

QPlatformServices *EglFSIntegration::services() const
{
    return m_services.data();
}

QPlatformNativeInterface *EglFSIntegration::nativeInterface() const
{
    return m_nativeInterface.data();
}

VtHandler *EglFSIntegration::vtHandler() const
{
    return m_vtHandler.data();
}

void EglFSIntegration::initialize()
{
    if (!egl_device_integration()->configurationFileName().isEmpty())
        egl_device_integration()->loadConfiguration(egl_device_integration()->configurationFileName());

    if (egl_device_integration()->needsLogind()) {
        // Connect to logind and take control because the device integration
        // demands it
        Logind *logind = Logind::instance();
        if (logind->isConnected()) {
            qCDebug(lcDeviceIntegration) << "logind connection already established";
            takeControl(true);
        } else {
            qCDebug(lcDeviceIntegration) << "logind connection not yet established";
            connect(logind, &Logind::connectedChanged,
                    this, &EglFSIntegration::takeControl,
                    Qt::QueuedConnection);
        }

        // Wait for logind setup
        QEventLoop *loop = new QEventLoop();
        connect(this, &EglFSIntegration::initialized,
                loop, &QEventLoop::quit);
        loop->exec();
        delete loop;
    } else {
        // proceed with synchronous initialization because logind is not needed
        // by the device integration
        actualInitialization();
    }
}

void EglFSIntegration::destroy()
{
    if (!egl_device_integration()->configurationFileName().isEmpty())
        egl_device_integration()->saveConfiguration(egl_device_integration()->configurationFileName());

    Q_FOREACH (QWindow *window, QGuiApplication::topLevelWindows())
        window->destroy();

    egl_device_integration()->screenDestroy();

    if (m_display != EGL_NO_DISPLAY)
        eglTerminate(m_display);

    egl_device_integration()->platformDestroy();
}

QAbstractEventDispatcher *EglFSIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformWindow *EglFSIntegration::createPlatformWindow(QWindow *window) const
{
    QWindowSystemInterface::flushWindowSystemEvents();

    QPlatformWindow *w = egl_device_integration()->createPlatformWindow(window);
    if (!w) {
        w = new EglFSWindow(window);
        static_cast<EglFSWindow *>(w)->create();
    }

    // Activate only the compositor window for the primary screen in order to
    // make keyboard input work
    if (window->type() != Qt::ToolTip && window->screen() == QGuiApplication::primaryScreen())
        w->requestActivateWindow();

    return w;
}

QPlatformBackingStore *EglFSIntegration::createPlatformBackingStore(QWindow *window) const
{
    QPlatformBackingStore *pbs = egl_device_integration()->createPlatformBackingStore(window);
    if (pbs)
        return pbs;

    OpenGLCompositorBackingStore *bs = new OpenGLCompositorBackingStore(window);
    if (!window->handle())
        window->create();
    static_cast<EglFSWindow *>(window->handle())->setBackingStore(bs);
    return bs;
}

QPlatformOpenGLContext *EglFSIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    QPlatformOpenGLContext *pctx = egl_device_integration()->createPlatformOpenGLContext(context);
    if (pctx)
        return pctx;

    EGLDisplay dpy = context->screen() ? static_cast<EglFSScreen *>(context->screen()->handle())->display() : display();
    QPlatformOpenGLContext *share = context->shareHandle();
    QVariant nativeHandle = context->nativeHandle();

    EglFSContext *ctx;
    QSurfaceFormat adjustedFormat = egl_device_integration()->surfaceFormatFor(context->format());
    if (nativeHandle.isNull()) {
        EGLConfig config = EglFSIntegration::chooseConfig(dpy, adjustedFormat);
        ctx = new EglFSContext(adjustedFormat, share, dpy, &config, QVariant());
    } else {
        ctx = new EglFSContext(adjustedFormat, share, dpy, 0, nativeHandle);
    }
    nativeHandle = QVariant::fromValue<QEGLNativeContext>(QEGLNativeContext(ctx->eglContext(), dpy));

    context->setNativeHandle(nativeHandle);
    return ctx;
}

QPlatformOffscreenSurface *EglFSIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
    EGLDisplay dpy = surface->screen() ? static_cast<EglFSScreen *>(surface->screen()->handle())->display() : display();
    QSurfaceFormat fmt = egl_device_integration()->surfaceFormatFor(surface->requestedFormat());
    if (egl_device_integration()->supportsPBuffers())
        return new EGLPbuffer(dpy, fmt, surface);
    else
        return new EglFSOffscreenWindow(dpy, fmt, surface);
    // Never return null. Multiple QWindows are not supported by this plugin.
}

bool EglFSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    // We assume that devices will have more and not less capabilities
    if (egl_device_integration()->hasCapability(cap))
        return true;

    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    case WindowManagement: return false;
    case RasterGLSurface: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

void EglFSIntegration::addScreen(QPlatformScreen *screen)
{
    screenAdded(screen);
}

void EglFSIntegration::removeScreen(QPlatformScreen *screen)
{
    destroyScreen(screen);
}

EGLConfig EglFSIntegration::chooseConfig(EGLDisplay display, const QSurfaceFormat &format)
{
    class Chooser : public EglConfigChooser {
    public:
        Chooser(EGLDisplay display)
            : EglConfigChooser(display) { }
        bool filterConfig(EGLConfig config) const Q_DECL_OVERRIDE {
            return egl_device_integration()->filterConfig(display(), config)
                    && EglConfigChooser::filterConfig(config);
        }
    };

    Chooser chooser(display);
    chooser.setSurfaceFormat(format);
    return chooser.chooseConfig();
}

void EglFSIntegration::takeControl(bool connected)
{
    if (!connected)
        return;

    Logind *logind = Logind::instance();

    disconnect(logind, &Logind::connectedChanged,
            this, &EglFSIntegration::takeControl);

    if (logind->hasSessionControl()) {
        qCDebug(lcDeviceIntegration) << "Session control already acquired via logind";
        actualInitialization();
    } else {
        qCDebug(lcDeviceIntegration) << "Take control of session via logind";
        logind->takeControl();
        connect(logind, &Logind::hasSessionControlChanged,
                this, &EglFSIntegration::actualInitialization,
                Qt::QueuedConnection);
    }
}

void EglFSIntegration::actualInitialization()
{
    qCDebug(lcDeviceIntegration) << "Initialization";

    Logind *logind = Logind::instance();
    disconnect(logind, &Logind::hasSessionControlChanged,
            this, &EglFSIntegration::actualInitialization);

    egl_device_integration()->platformInit();

    m_display = eglGetDisplay(nativeDisplay());
    if (Q_UNLIKELY(m_display == EGL_NO_DISPLAY)) {
        qFatal("Failed to open EGL display");
        return;
    }

    EGLint major, minor;
    if (Q_UNLIKELY(!eglInitialize(m_display, &major, &minor))) {
        qFatal("Failed to initialize EGL display");
        return;
    }

    QString icStr = QPlatformInputContextFactory::requested();
    if (icStr.isNull())
        icStr = QLatin1String("compose");
    m_inputContext = QPlatformInputContextFactory::create(icStr);

    if (egl_device_integration()->usesVtHandler())
        m_vtHandler.reset(new VtHandler);

    if (!egl_device_integration()->handlesInput())
        m_liHandler = new Platform::LibInputManager(this);

    if (egl_device_integration()->usesDefaultScreen())
        addScreen(new EglFSScreen(display()));
    else
        egl_device_integration()->screenInit();

    // Set the first screen as primary
    QScreen *firstScreen = QGuiApplication::screens().at(0);
    if (firstScreen && firstScreen->handle())
        setPrimaryScreen(firstScreen->handle());

    Q_EMIT initialized();
}

} // namespace Platform

} // namespace GreenIsland
