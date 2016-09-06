/****************************************************************************
 * This file is part of Hawaii.
 *
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

#ifndef GREENISLAND_EGLFSINTEGRATION_H
#define GREENISLAND_EGLFSINTEGRATION_H

#include <QtGui/qpa/qplatformfontdatabase.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/qpa/qplatformscreen.h>
#include <QtGui/qpa/qplatformservices.h>

#include <GreenIsland/platform/greenislandplatform_export.h>
#include <GreenIsland/Platform/EglFSNativeInterface>
#include <GreenIsland/Platform/VtHandler>

#include <EGL/egl.h>

namespace GreenIsland {

namespace Platform {

class EglFSContext;
class EglFSWindow;
class LibInputManager;

class GREENISLANDPLATFORM_EXPORT EglFSIntegration : public QObject, public QPlatformIntegration
{
    Q_OBJECT
public:
    EglFSIntegration();

    EGLDisplay display() const;
    EGLNativeDisplayType nativeDisplay() const;

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

    void addScreen(QPlatformScreen *screen);
    void removeScreen(QPlatformScreen *screen);

    static EGLConfig chooseConfig(EGLDisplay display, const QSurfaceFormat &format);

Q_SIGNALS:
    void initialized();

private:
    EGLDisplay m_display;

    QScopedPointer<EglFSNativeInterface> m_nativeInterface;
    QPlatformInputContext *m_inputContext;
    QScopedPointer<QPlatformFontDatabase> m_fontDatabase;
    QScopedPointer<QPlatformServices> m_services;
    QScopedPointer<VtHandler> m_vtHandler;
    LibInputManager *m_liHandler;

private Q_SLOTS:
    void takeControl(bool connected);
    void actualInitialization();
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSINTEGRATION_H
