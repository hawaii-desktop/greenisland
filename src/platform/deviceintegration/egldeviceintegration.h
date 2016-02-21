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
