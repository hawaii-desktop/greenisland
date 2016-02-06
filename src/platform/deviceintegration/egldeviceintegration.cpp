/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtCore/private/qcore_unix_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/private/qguiapplication_p.h>

#include "logging.h"
#include "eglconvenience/eglconvenience.h"
#include "deviceintegration/deviceintegration_p.h"
#include "deviceintegration/egldeviceintegration.h"
#include "deviceintegration/eglfscursor.h"
#include "deviceintegration/eglfsintegration.h"

#if defined(Q_OS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#endif

namespace GreenIsland {

namespace Platform {

Q_GLOBAL_STATIC(DeviceIntegration, gDeviceIntegration)

EGLDeviceIntegration *egl_device_integration()
{
    return gDeviceIntegration()->integration();
}

static int framebuffer = -1;

EGLDeviceIntegration::~EGLDeviceIntegration()
{
}

EGLNativeDisplayType EGLDeviceIntegration::platformDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

QByteArray EGLDeviceIntegration::fbDeviceName() const
{
    QByteArray fbDev = qgetenv("GREENISLAND_QPA_FB");
    if (fbDev.isEmpty())
        fbDev = QByteArrayLiteral("/dev/fb0");

    return fbDev;
}

int EGLDeviceIntegration::framebufferIndex() const
{
    int fbIndex = 0;
#ifndef QT_NO_REGULAREXPRESSION
    QRegularExpression fbIndexRx(QLatin1String("fb(\\d+)"));
    QRegularExpressionMatch match = fbIndexRx.match(QString::fromLocal8Bit(fbDeviceName()));
    if (match.hasMatch())
        fbIndex = match.captured(1).toInt();
#endif
    return fbIndex;
}

void EGLDeviceIntegration::platformInit()
{
    QByteArray fbDev = fbDeviceName();

    framebuffer = qt_safe_open(fbDev, O_RDONLY);

    if (Q_UNLIKELY(framebuffer == -1)) {
        qCWarning(lcDeviceIntegration, "Failed to open %s", fbDev.constData());
        qFatal("Can't continue without a display");
        return;
    }

#ifdef FBIOBLANK
    ioctl(framebuffer, FBIOBLANK, VESA_NO_BLANKING);
#endif
}

void EGLDeviceIntegration::platformDestroy()
{
    if (framebuffer != -1)
        close(framebuffer);
}

bool EGLDeviceIntegration::handlesInput()
{
    return false;
}

bool EGLDeviceIntegration::usesVtHandler()
{
    return true;
}

bool EGLDeviceIntegration::usesDefaultScreen()
{
    return true;
}

void EGLDeviceIntegration::screenInit()
{
    // Nothing to do here. Called only when usesDefaultScreen is false.
}

void EGLDeviceIntegration::screenDestroy()
{
    QGuiApplication *app = qGuiApp;
    EglFSIntegration *platformIntegration = static_cast<EglFSIntegration *>(
                QGuiApplicationPrivate::platformIntegration());
    while (!app->screens().isEmpty())
        platformIntegration->removeScreen(app->screens().last()->handle());
}

QSizeF EGLDeviceIntegration::physicalScreenSize() const
{
    return EglUtils::physicalScreenSizeFromFb(framebuffer, screenSize());
}

QSize EGLDeviceIntegration::screenSize() const
{
    return EglUtils::screenSizeFromFb(framebuffer);
}

QDpi EGLDeviceIntegration::logicalDpi() const
{
    const QSizeF ps = physicalScreenSize();
    const QSize s = screenSize();

    if (!ps.isEmpty() && !s.isEmpty())
        return QDpi(25.4 * s.width() / ps.width(),
                    25.4 * s.height() / ps.height());
    else
        return QDpi(100, 100);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
qreal EGLDeviceIntegration::pixelDensity() const
{
    return qRound(logicalDpi().first / qreal(100));
}
#endif

Qt::ScreenOrientation EGLDeviceIntegration::nativeOrientation() const
{
    return Qt::PrimaryOrientation;
}

Qt::ScreenOrientation EGLDeviceIntegration::orientation() const
{
    return Qt::PrimaryOrientation;
}

int EGLDeviceIntegration::screenDepth() const
{
    return EglUtils::screenDepthFromFb(framebuffer);
}

QImage::Format EGLDeviceIntegration::screenFormat() const
{
    return screenDepth() == 16 ? QImage::Format_RGB16 : QImage::Format_RGB32;
}

qreal EGLDeviceIntegration::refreshRate() const
{
    return EglUtils::refreshRateFromFb(framebuffer);
}

QSurfaceFormat EGLDeviceIntegration::surfaceFormatFor(const QSurfaceFormat &inputFormat) const
{
    QSurfaceFormat format = inputFormat;

    static const bool force888 = qEnvironmentVariableIntValue("GREENISLAND_QPA_FORCE888");
    if (force888) {
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
    }

    return format;
}

bool EGLDeviceIntegration::filterConfig(EGLDisplay, EGLConfig) const
{
    return true;
}

QPlatformWindow *EGLDeviceIntegration::createPlatformWindow(QWindow *window)
{
    Q_UNUSED(window);
    return Q_NULLPTR;
}

QPlatformBackingStore *EGLDeviceIntegration::createPlatformBackingStore(QWindow *window)
{
    Q_UNUSED(window);
    return Q_NULLPTR;
}

QPlatformOpenGLContext *EGLDeviceIntegration::createPlatformOpenGLContext(QOpenGLContext *context)
{
    Q_UNUSED(context);
    return Q_NULLPTR;
}

EGLNativeWindowType EGLDeviceIntegration::createNativeWindow(QPlatformWindow *platformWindow,
                                                             const QSize &size,
                                                             const QSurfaceFormat &format)
{
    Q_UNUSED(platformWindow);
    Q_UNUSED(size);
    Q_UNUSED(format);
    return 0;
}

EGLNativeWindowType EGLDeviceIntegration::createNativeOffscreenWindow(const QSurfaceFormat &format)
{
    Q_UNUSED(format);
    return 0;
}

void EGLDeviceIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    Q_UNUSED(window);
}

bool EGLDeviceIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    Q_UNUSED(cap);
    return false;
}

QPlatformCursor *EGLDeviceIntegration::createCursor(QPlatformScreen *screen) const
{
    return new EglFSCursor(screen);
}

void EGLDeviceIntegration::waitForVSync(QPlatformSurface *surface) const
{
    Q_UNUSED(surface);

#if defined(FBIO_WAITFORVSYNC)
    static const bool forceSync = qEnvironmentVariableIntValue("GREENISLAND_QPA_FORCEVSYNC");
    if (forceSync && framebuffer != -1) {
        int arg = 0;
        if (ioctl(framebuffer, FBIO_WAITFORVSYNC, &arg) == -1)
            qCWarning(lcDeviceIntegration, "Could not wait for vsync");
    }
#endif
}

void EGLDeviceIntegration::presentBuffer(QPlatformSurface *surface)
{
    Q_UNUSED(surface);
}

bool EGLDeviceIntegration::supportsPBuffers() const
{
    return true;
}

void *EGLDeviceIntegration::wlDisplay() const
{
    return Q_NULLPTR;
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_egldeviceintegration.cpp"
