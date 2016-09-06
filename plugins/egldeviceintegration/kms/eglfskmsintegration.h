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

#ifndef GREENISLAND_EGLFSKMSINTEGRATION_H
#define GREENISLAND_EGLFSKMSINTEGRATION_H

#include <QtCore/QMap>
#include <QtCore/QVariant>

#include <GreenIsland/Platform/EGLDeviceIntegration>

namespace GreenIsland {

namespace Platform {

class EglFSKmsDevice;

class EglFSKmsIntegration : public EGLDeviceIntegration
{
public:
    EglFSKmsIntegration();

    void platformInit() Q_DECL_OVERRIDE;
    void platformDestroy() Q_DECL_OVERRIDE;
    void loadConfiguration(const QString &fileName) Q_DECL_OVERRIDE;
    void saveConfiguration(const QString &fileName) Q_DECL_OVERRIDE;
    bool needsLogind() Q_DECL_OVERRIDE;
    EGLNativeDisplayType platformDisplay() const Q_DECL_OVERRIDE;
    bool usesDefaultScreen() Q_DECL_OVERRIDE;
    void screenInit() Q_DECL_OVERRIDE;
    QSurfaceFormat surfaceFormatFor(const QSurfaceFormat &inputFormat) const Q_DECL_OVERRIDE;
    QPlatformWindow *createPlatformWindow(QWindow *window) Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeWindow(QPlatformWindow *platformWindow,
                                           const QSize &size,
                                           const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeOffscreenWindow(const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    void destroyNativeWindow(EGLNativeWindowType window) Q_DECL_OVERRIDE;
    bool hasCapability(QPlatformIntegration::Capability cap) const Q_DECL_OVERRIDE;
    QPlatformCursor *createCursor(QPlatformScreen *screen) const Q_DECL_OVERRIDE;
    bool isResizingSurface(QPlatformSurface *surface) const Q_DECL_OVERRIDE;
    void waitForVSync(QPlatformSurface *surface) const Q_DECL_OVERRIDE;
    void resizeSurface(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    void presentBuffer(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    bool supportsPBuffers() const Q_DECL_OVERRIDE;

    bool hwCursor() const;
    bool separateScreens() const;
    QMap<QString, QVariantMap> outputSettings() const;

private:
    EglFSKmsDevice *m_device;
    bool m_hwCursor;
    bool m_pbuffers;
    bool m_separateScreens;
    QString m_devicePath;
    QMap<QString, QVariantMap> m_outputSettings;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSKMSINTEGRATION_H
