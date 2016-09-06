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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/QScreen>

#include <GreenIsland/Platform/EglFSCursor>
#include <GreenIsland/Platform/EglFSWindow>
#include <GreenIsland/Platform/Udev>
#include <GreenIsland/Platform/UdevEnumerate>

#include "eglfskmsintegration.h"
#include "eglfskmsdevice.h"
#include "eglfskmsscreen.h"
#include "eglfskmscursor.h"
#include "eglfskmswindow.h"

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
    // Autodetect DRM device unless it's specified by the configuration
    if (m_devicePath.isEmpty()) {
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
        qCInfo(lcKms) << "Using autodetected" << m_devicePath;
    } else {
        qCInfo(lcKms) << "Using" << m_devicePath << "from configuration";
    }

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

void EglFSKmsIntegration::loadConfiguration(const QString &fileName)
{
    qCInfo(lcKms) << "Loading configuration from" << fileName;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcKms) << "Could not open configuration file"
                         << fileName << "for reading";
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject() || !doc.object().contains(QStringLiteral("kms"))) {
        qCWarning(lcKms) << "Invalid configuration file" << fileName
                         << "- no top-level JSON object";
        return;
    }

    const QJsonObject object = doc.object().value(QStringLiteral("kms")).toObject();

    m_hwCursor = object.value(QStringLiteral("hwcursor")).toBool(m_hwCursor);
    m_pbuffers = object.value(QStringLiteral("pbuffers")).toBool(m_pbuffers);
    m_devicePath = object.value(QStringLiteral("device")).toString();
    m_separateScreens = object.value(QStringLiteral("separateScreens")).toBool(m_separateScreens);

    const QJsonArray outputs = object.value(QStringLiteral("outputs")).toArray();
    for (int i = 0; i < outputs.size(); i++) {
        const QVariantMap outputSettings = outputs.at(i).toObject().toVariantMap();

        if (outputSettings.contains(QStringLiteral("name"))) {
            const QString name = outputSettings.value(QStringLiteral("name")).toString();

            if (m_outputSettings.contains(name))
                qCDebug(lcKms) << "Output" << name << "configured multiple times!";

            m_outputSettings.insert(name, outputSettings);
        }
    }

    qCDebug(lcKms) << "Configuration:\n"
                   << "\tdevice:" << m_devicePath << "\n"
                   << "\thwcursor:" << m_hwCursor << "\n"
                   << "\tpbuffers:" << m_pbuffers << "\n"
                   << "\tseparateScreens:" << m_separateScreens << "\n"
                   << "\toutputs:" << m_outputSettings;
}

void EglFSKmsIntegration::saveConfiguration(const QString &fileName)
{
    qCInfo(lcKms) << "Saving configuration to" << fileName;

    QFileInfo fileInfo(fileName);
    fileInfo.absoluteDir().mkpath(QLatin1String("."));

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite)) {
        qCWarning(lcKms) << "Could not open configuration file"
                         << fileName << "for writing";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    QJsonObject rootObject;
    if (doc.isObject())
        rootObject = doc.object();

    file.seek(0);

    // Save device only if it was specified by the original configuration,
    // this will disable autodetect next time the configuration is
    // loaded: comes in handy on systems with multiple GPUs where the
    // user wants to pick a specific one instead of the first that
    // is autodetected
    QVariantMap map;
    map[QStringLiteral("hwcursor")] = m_hwCursor;
    map[QStringLiteral("pbuffers")] = m_pbuffers;
    const QString devicePath = rootObject.value(QStringLiteral("kms")).toObject().value(QStringLiteral("device")).toString();
    if (!devicePath.isEmpty())
        map[QStringLiteral("device")] = devicePath;
    map[QStringLiteral("separateScreens")] = m_separateScreens;

    QJsonArray outputs;
    Q_FOREACH (const QScreen *screen, QGuiApplication::screens()) {
        EglFSKmsScreen *kmsScreen = static_cast<EglFSKmsScreen *>(screen->handle());
        if (!kmsScreen)
            continue;

        QVariantMap output;
        output[QStringLiteral("name")] = screen->name();
        if (kmsScreen->currentMode() == kmsScreen->preferredMode())
            output[QStringLiteral("mode")] = QStringLiteral("preferred");
        else
            output[QStringLiteral("mode")] = QStringLiteral("%1x%2")
                    .arg(QString::number(kmsScreen->geometry().size().width()))
                    .arg(QString::number(kmsScreen->geometry().size().height()));

        outputs.append(QJsonValue::fromVariant(output));
    }
    map[QStringLiteral("outputs")] = outputs;

    rootObject[QStringLiteral("kms")] = QJsonValue::fromVariant(map);
    doc.setObject(rootObject);

    file.write(doc.toJson());
    file.close();
}

bool EglFSKmsIntegration::needsLogind()
{
    return true;
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

QPlatformWindow *EglFSKmsIntegration::createPlatformWindow(QWindow *window)
{
    EglFSKmsWindow *kmsWindow = new EglFSKmsWindow(window);
    kmsWindow->create();
    return kmsWindow;
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

bool EglFSKmsIntegration::isResizingSurface(QPlatformSurface *surface) const
{
    QWindow *window = static_cast<QWindow *>(surface->surface());
    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(window->screen()->handle());

    return screen->isResizingSurface();
}

void EglFSKmsIntegration::waitForVSync(QPlatformSurface *surface) const
{
    QWindow *window = static_cast<QWindow *>(surface->surface());
    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(window->screen()->handle());

    screen->waitForFlip();
}

void EglFSKmsIntegration::resizeSurface(QPlatformSurface *surface)
{
    QWindow *w = static_cast<QWindow *>(surface->surface());
    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(w->screen()->handle());
    EglFSKmsWindow *window = static_cast<EglFSKmsWindow *>(w->handle());

    screen->resizeSurface();
    window->resizeSurface();
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

} // namespace Platform

} // namespace GreenIsland
