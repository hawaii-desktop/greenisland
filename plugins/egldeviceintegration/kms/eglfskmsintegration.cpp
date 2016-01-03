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
