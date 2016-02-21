/****************************************************************************
**
** Copyright (C) 2015-2016 Pier Luigi Fiorini
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
