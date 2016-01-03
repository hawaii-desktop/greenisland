/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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
    EGLNativeDisplayType platformDisplay() const Q_DECL_OVERRIDE;
    bool usesDefaultScreen() Q_DECL_OVERRIDE;
    void screenInit() Q_DECL_OVERRIDE;
    QSurfaceFormat surfaceFormatFor(const QSurfaceFormat &inputFormat) const Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeWindow(QPlatformWindow *platformWindow,
                                           const QSize &size,
                                           const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeOffscreenWindow(const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    void destroyNativeWindow(EGLNativeWindowType window) Q_DECL_OVERRIDE;
    bool hasCapability(QPlatformIntegration::Capability cap) const Q_DECL_OVERRIDE;
    QPlatformCursor *createCursor(QPlatformScreen *screen) const Q_DECL_OVERRIDE;
    void waitForVSync(QPlatformSurface *surface) const Q_DECL_OVERRIDE;
    void presentBuffer(QPlatformSurface *surface) Q_DECL_OVERRIDE;
    bool supportsPBuffers() const Q_DECL_OVERRIDE;

    bool hwCursor() const;
    bool separateScreens() const;
    QMap<QString, QVariantMap> outputSettings() const;

private:
    void loadConfig();

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
