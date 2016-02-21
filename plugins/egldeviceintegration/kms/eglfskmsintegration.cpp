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

#include <QtCore/QLoggingCategory>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/QScreen>

#include <GreenIsland/Platform/EglFSCursor>
#include <GreenIsland/Platform/Udev>
#include <GreenIsland/Platform/UdevEnumerate>

#include "eglfskmsintegration.h"
#include "eglfskmsdevice.h"
#include "eglfskmsscreen.h"
#include "eglfskmscursor.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

namespace GreenIsland {

namespace Platform {

Q_LOGGING_CATEGORY(lcKms, "greenisland.qpa.kms")

QMutex EglFSKmsScreen::m_waitForFlipMutex;

EglFSKmsIntegration::EglFSKmsIntegration()
    : m_device(Q_NULLPTR)
    , m_hwCursor(true)
    , m_pbuffers(false)
    , m_separateScreens(false)
{
}

void EglFSKmsIntegration::platformInit()
{
    loadConfig();

    Udev *udev = new Udev;
    UdevEnumerate *udevEnumerate = new UdevEnumerate(UdevDevice::PrimaryVideoDevice |
                                                     UdevDevice::GenericVideoDevice, udev);
    QList<UdevDevice *> devices = udevEnumerate->scan();
    qCDebug(lcKms) << "Found the following video devices:";
    Q_FOREACH (UdevDevice *device, devices)
        qCDebug(lcKms) << '\t' << device->deviceNode().toUtf8().constData();

    delete udevEnumerate;
    delete udev;

    if (devices.isEmpty())
        qFatal("Could not find DRM device!");

    m_devicePath = devices.first()->deviceNode();
    qCDebug(lcKms) << "Using" << m_devicePath;

    m_device = new EglFSKmsDevice(this, m_devicePath);
    if (!m_device->open())
        qFatal("Could not open device %s - aborting!", qPrintable(m_devicePath));
}

void EglFSKmsIntegration::platformDestroy()
{
    m_device->close();
    delete m_device;
    m_device = Q_NULLPTR;
}

EGLNativeDisplayType EglFSKmsIntegration::platformDisplay() const
{
    Q_ASSERT(m_device);
    return reinterpret_cast<EGLNativeDisplayType>(m_device->device());
}

bool EglFSKmsIntegration::usesDefaultScreen()
{
    return false;
}

void EglFSKmsIntegration::screenInit()
{
    m_device->createScreens();
}

QSurfaceFormat EglFSKmsIntegration::surfaceFormatFor(const QSurfaceFormat &inputFormat) const
{
    QSurfaceFormat format(inputFormat);
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    return format;
}

EGLNativeWindowType EglFSKmsIntegration::createNativeWindow(QPlatformWindow *platformWindow,
                                                            const QSize &size,
                                                            const QSurfaceFormat &format)
{
    Q_UNUSED(size);
    Q_UNUSED(format);

    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(platformWindow->screen());
    if (screen->surface()) {
        qWarning("Only single window per screen supported!");
        return 0;
    }

    return reinterpret_cast<EGLNativeWindowType>(screen->createSurface());
}

EGLNativeWindowType EglFSKmsIntegration::createNativeOffscreenWindow(const QSurfaceFormat &format)
{
    Q_UNUSED(format);
    Q_ASSERT(m_device);

    qCDebug(lcKms) << "Creating native off screen window";
    gbm_surface *surface = gbm_surface_create(m_device->device(),
                                              1, 1,
                                              GBM_FORMAT_XRGB8888,
                                              GBM_BO_USE_RENDERING);

    return reinterpret_cast<EGLNativeWindowType>(surface);
}

void EglFSKmsIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    gbm_surface *surface = reinterpret_cast<gbm_surface *>(window);
    gbm_surface_destroy(surface);
}

bool EglFSKmsIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case QPlatformIntegration::ThreadedPixmaps:
    case QPlatformIntegration::OpenGL:
    case QPlatformIntegration::ThreadedOpenGL:
        return true;
    default:
        return false;
    }
}

QPlatformCursor *EglFSKmsIntegration::createCursor(QPlatformScreen *screen) const
{
    if (m_hwCursor)
        return Q_NULLPTR;
    else
        return new EglFSCursor(screen);
}

void EglFSKmsIntegration::waitForVSync(QPlatformSurface *surface) const
{
    QWindow *window = static_cast<QWindow *>(surface->surface());
    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(window->screen()->handle());

    screen->waitForFlip();
}

void EglFSKmsIntegration::presentBuffer(QPlatformSurface *surface)
{
    QWindow *window = static_cast<QWindow *>(surface->surface());
    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(window->screen()->handle());

    screen->flip();
}

bool EglFSKmsIntegration::supportsPBuffers() const
{
    return m_pbuffers;
}

bool EglFSKmsIntegration::hwCursor() const
{
    return m_hwCursor;
}

bool EglFSKmsIntegration::separateScreens() const
{
    return m_separateScreens;
}

QMap<QString, QVariantMap> EglFSKmsIntegration::outputSettings() const
{
    return m_outputSettings;
}

void EglFSKmsIntegration::loadConfig()
{
    static QByteArray json = qgetenv("QT_QPA_EGLFS_KMS_CONFIG");
    if (json.isEmpty())
        return;

    qCDebug(lcKms) << "Loading KMS setup from" << json;

    QFile file(QString::fromUtf8(json));
    if (!file.open(QFile::ReadOnly)) {
        qCDebug(lcKms) << "Could not open config file"
                       << json << "for reading";
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        qCDebug(lcKms) << "Invalid config file" << json
                       << "- no top-level JSON object";
        return;
    }

    const QJsonObject object = doc.object();

    m_hwCursor = object.value(QStringLiteral("hwcursor")).toBool(m_hwCursor);
    m_pbuffers = object.value(QStringLiteral("pbuffers")).toBool(m_pbuffers);
    m_devicePath = object.value(QStringLiteral("device")).toString();
    m_separateScreens = object.value(QStringLiteral("separateScreens")).toBool(m_separateScreens);

    const QJsonArray outputs = object.value(QStringLiteral("outputs")).toArray();
    for (int i = 0; i < outputs.size(); i++) {
        const QVariantMap outputSettings = outputs.at(i).toObject().toVariantMap();

        if (outputSettings.contains(QStringLiteral("name"))) {
            const QString name = outputSettings.value(QStringLiteral("name")).toString();

            if (m_outputSettings.contains(name)) {
                qCDebug(lcKms) << "Output" << name << "configured multiple times!";
            }

            m_outputSettings.insert(name, outputSettings);
        }
    }

    qCDebug(lcKms) << "Configuration:\n"
                   << "\thwcursor:" << m_hwCursor << "\n"
                   << "\tpbuffers:" << m_pbuffers << "\n"
                   << "\tseparateScreens:" << m_separateScreens << "\n"
                   << "\toutputs:" << m_outputSettings;
}

} // namespace Platform

} // namespace GreenIsland
