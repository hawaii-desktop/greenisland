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

#ifndef GREENISLAND_EGLFSINTEGRATION_H
#define GREENISLAND_EGLFSINTEGRATION_H

#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qplatformscreen.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <EGL/egl.h>

namespace GreenIsland {

namespace Platform {

class EglFSContext;
class EglFSWindow;
class LibInputManager;
class VtHandler;

class GREENISLANDPLATFORM_EXPORT EglFSIntegration : public QPlatformIntegration, public QPlatformNativeInterface
{
public:
    EglFSIntegration();

    EGLDisplay display() const;

    QPlatformInputContext *inputContext() const Q_DECL_OVERRIDE;
    QPlatformFontDatabase *fontDatabase() const Q_DECL_OVERRIDE;
    QPlatformServices *services() const Q_DECL_OVERRIDE;
    QPlatformNativeInterface *nativeInterface() const Q_DECL_OVERRIDE;
    VtHandler *vtHandler() const;

    void initialize() Q_DECL_OVERRIDE;
    void destroy() Q_DECL_OVERRIDE;

    QAbstractEventDispatcher *createEventDispatcher() const Q_DECL_OVERRIDE;
    QPlatformWindow *createPlatformWindow(QWindow *window) const Q_DECL_OVERRIDE;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const Q_DECL_OVERRIDE;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const Q_DECL_OVERRIDE;
    QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const Q_DECL_OVERRIDE;

    bool hasCapability(QPlatformIntegration::Capability cap) const Q_DECL_OVERRIDE;

    // QPlatformNativeInterface
    void *nativeResourceForIntegration(const QByteArray &resource) Q_DECL_OVERRIDE;
    void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen) Q_DECL_OVERRIDE;
    void *nativeResourceForWindow(const QByteArray &resource, QWindow *window) Q_DECL_OVERRIDE;
    void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) Q_DECL_OVERRIDE;
    NativeResourceForContextFunction nativeResourceFunctionForContext(const QByteArray &resource) Q_DECL_OVERRIDE;

    QFunctionPointer platformFunction(const QByteArray &function) const Q_DECL_OVERRIDE;

    void addScreen(QPlatformScreen *screen);
    void removeScreen(QPlatformScreen *screen);

    static EGLConfig chooseConfig(EGLDisplay display, const QSurfaceFormat &format);

private:
    EGLDisplay m_display;
    QPlatformInputContext *m_inputContext;
    QScopedPointer<QPlatformFontDatabase> m_fontDatabase;
    QScopedPointer<QPlatformServices> m_services;
    QScopedPointer<VtHandler> m_vtHandler;
    LibInputManager *m_liHandler;

    EGLNativeDisplayType nativeDisplay() const;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSINTEGRATION_H
