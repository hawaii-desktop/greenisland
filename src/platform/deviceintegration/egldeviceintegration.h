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

#ifndef GREENISLAND_EGLDEVICEINTEGRATION_H
#define GREENISLAND_EGLDEVICEINTEGRATION_H

#include <QtCore/QString>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QImage>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/qpa/qplatformscreen.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <EGL/egl.h>

class QPlatformSurface;

namespace GreenIsland {

namespace Platform {

#define GreenIslandDeviceIntegrationFactoryInterface_iid "org.hawaiios.GreenIsland.DeviceIntegrationFactoryInterface.1.0"

class GREENISLANDPLATFORM_EXPORT EGLDeviceIntegration
{
public:
    virtual ~EGLDeviceIntegration();

    virtual EGLNativeDisplayType platformDisplay() const;

    virtual QByteArray fbDeviceName() const;
    virtual int framebufferIndex() const;

    virtual void platformInit();
    virtual void platformDestroy();

    virtual bool handlesInput();
    virtual bool usesVtHandler();
    virtual bool usesDefaultScreen();

    virtual void screenInit();
    virtual void screenDestroy();

    virtual QSizeF physicalScreenSize() const;
    virtual QSize screenSize() const;
    virtual QDpi logicalDpi() const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    virtual qreal pixelDensity() const;
#endif
    virtual Qt::ScreenOrientation nativeOrientation() const;
    virtual Qt::ScreenOrientation orientation() const;
    virtual int screenDepth() const;
    virtual QImage::Format screenFormat() const;
    virtual qreal refreshRate() const;

    virtual QSurfaceFormat surfaceFormatFor(const QSurfaceFormat &inputFormat) const;

    virtual QPlatformWindow *createPlatformWindow(QWindow *window);
    virtual QPlatformBackingStore *createPlatformBackingStore(QWindow *window);
    virtual QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context);

    virtual EGLNativeWindowType createNativeWindow(QPlatformWindow *platformWindow,
                                                   const QSize &size,
                                                   const QSurfaceFormat &format);
    virtual EGLNativeWindowType createNativeOffscreenWindow(const QSurfaceFormat &format);
    virtual void destroyNativeWindow(EGLNativeWindowType window);

    virtual bool hasCapability(QPlatformIntegration::Capability cap) const;

    virtual QPlatformCursor *createCursor(QPlatformScreen *screen) const;

    virtual bool filterConfig(EGLDisplay display, EGLConfig config) const;

    virtual void waitForVSync(QPlatformSurface *surface) const;
    virtual void presentBuffer(QPlatformSurface *surface);

    virtual bool supportsPBuffers() const;

    virtual void *wlDisplay() const;
};

class GREENISLANDPLATFORM_EXPORT EGLDeviceIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    virtual EGLDeviceIntegration *create() = 0;
};

GREENISLANDPLATFORM_EXPORT EGLDeviceIntegration *egl_device_integration();

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLDEVICEINTEGRATION_H
