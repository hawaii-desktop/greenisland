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
#include <QtCore/private/qcore_unix_p.h>
#include <QtGui/private/qguiapplication_p.h>

#include <GreenIsland/Platform/EGLDeviceIntegration>
#include <GreenIsland/Platform/EglFSIntegration>

#include "eglfskmsdevice.h"
#include "eglfskmsscreen.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

namespace GreenIsland {

namespace Platform {

Q_DECLARE_LOGGING_CATEGORY(lcKms)

enum OutputConfiguration {
    OutputConfigOff,
    OutputConfigPreferred,
    OutputConfigCurrent,
    OutputConfigMode,
    OutputConfigModeline
};

int EglFSKmsDevice::crtcForConnector(drmModeResPtr resources, drmModeConnectorPtr connector)
{
    for (int i = 0; i < connector->count_encoders; i++) {
        drmModeEncoderPtr encoder = drmModeGetEncoder(m_dri_fd, connector->encoders[i]);
        if (!encoder) {
            qCWarning(lcKms, "Failed to get encoder");
            continue;
        }

        quint32 possibleCrtcs = encoder->possible_crtcs;
        drmModeFreeEncoder(encoder);

        for (int j = 0; j < resources->count_crtcs; j++) {
            bool isPossible = possibleCrtcs & (1 << j);
            bool isAvailable = !(m_crtc_allocator & 1 << resources->crtcs[j]);

            if (isPossible && isAvailable)
                return j;
        }
    }

    return -1;
}

static const char * const connector_type_names[] = {
    "None",
    "VGA",
    "DVI",
    "DVI",
    "DVI",
    "Composite",
    "TV",
    "LVDS",
    "CTV",
    "DIN",
    "DP",
    "HDMI",
    "HDMI",
    "TV",
    "eDP",
};

static QByteArray nameForConnector(const drmModeConnectorPtr connector)
{
    QByteArray connectorName("UNKNOWN");

    if (connector->connector_type < ARRAY_LENGTH(connector_type_names))
        connectorName = connector_type_names[connector->connector_type];

    connectorName += QByteArray::number(connector->connector_type_id);

    return connectorName;
}

static bool parseModeline(const QByteArray &text, drmModeModeInfoPtr mode)
{
    char hsync[16];
    char vsync[16];
    float fclock;

    mode->type = DRM_MODE_TYPE_USERDEF;
    mode->hskew = 0;
    mode->vscan = 0;
    mode->vrefresh = 0;
    mode->flags = 0;

    if (sscanf(text.constData(), "%f %hd %hd %hd %hd %hd %hd %hd %hd %15s %15s",
               &fclock,
               &mode->hdisplay,
               &mode->hsync_start,
               &mode->hsync_end,
               &mode->htotal,
               &mode->vdisplay,
               &mode->vsync_start,
               &mode->vsync_end,
               &mode->vtotal, hsync, vsync) != 11)
        return false;

    mode->clock = fclock * 1000;

    if (strcmp(hsync, "+hsync") == 0)
        mode->flags |= DRM_MODE_FLAG_PHSYNC;
    else if (strcmp(hsync, "-hsync") == 0)
        mode->flags |= DRM_MODE_FLAG_NHSYNC;
    else
        return false;

    if (strcmp(vsync, "+vsync") == 0)
        mode->flags |= DRM_MODE_FLAG_PVSYNC;
    else if (strcmp(vsync, "-vsync") == 0)
        mode->flags |= DRM_MODE_FLAG_NVSYNC;
    else
        return false;

    return true;
}

EglFSKmsScreen *EglFSKmsDevice::screenForConnector(drmModeResPtr resources, drmModeConnectorPtr connector, QPoint pos)
{
    const QByteArray connectorName = nameForConnector(connector);

    const int crtc = crtcForConnector(resources, connector);
    if (crtc < 0) {
        qCWarning(lcKms, ) << "No usable crtc/encoder pair for connector" << connectorName;
        return Q_NULLPTR;
    }

    OutputConfiguration configuration;
    QSize configurationSize;
    drmModeModeInfo configurationModeline;

    const QByteArray mode = m_integration->outputSettings().value(QString::fromUtf8(connectorName))
            .value(QStringLiteral("mode"), QStringLiteral("preferred")).toByteArray().toLower();
    if (mode == "off") {
        configuration = OutputConfigOff;
    } else if (mode == "preferred") {
        configuration = OutputConfigPreferred;
    } else if (mode == "current") {
        configuration = OutputConfigCurrent;
    } else if (sscanf(mode.constData(), "%dx%d", &configurationSize.rwidth(), &configurationSize.rheight()) == 2) {
        configuration = OutputConfigMode;
    } else if (parseModeline(mode, &configurationModeline)) {
        configuration = OutputConfigModeline;
    } else {
        qCWarning(lcKms, "Invalid mode \"%s\" for output %s", mode.constData(), connectorName.constData());
        configuration = OutputConfigPreferred;
    }

    const uint32_t crtc_id = resources->crtcs[crtc];

    if (configuration == OutputConfigOff) {
        qCDebug(lcKms) << "Turning off output" << connectorName;
        drmModeSetCrtc(m_dri_fd, crtc_id, 0, 0, 0, 0, 0, Q_NULLPTR);
        return Q_NULLPTR;
    }

    // Get the current mode on the current crtc
    drmModeModeInfo crtc_mode;
    memset(&crtc_mode, 0, sizeof crtc_mode);
    if (drmModeEncoderPtr encoder = drmModeGetEncoder(m_dri_fd, connector->connector_id)) {
        drmModeCrtcPtr crtc = drmModeGetCrtc(m_dri_fd, encoder->crtc_id);
        drmModeFreeEncoder(encoder);

        if (!crtc)
            return Q_NULLPTR;

        if (crtc->mode_valid)
            crtc_mode = crtc->mode;

        drmModeFreeCrtc(crtc);
    }

    QList<drmModeModeInfo> modes;
    modes.reserve(connector->count_modes);
    qCDebug(lcKms) << connectorName << "mode count:" << connector->count_modes;
    for (int i = 0; i < connector->count_modes; i++) {
        const drmModeModeInfo &mode = connector->modes[i];
        qCDebug(lcKms) << "mode" << i << mode.hdisplay << "x" << mode.vdisplay
                       << "@" << mode.vrefresh << "hz";
        modes << connector->modes[i];
    }

    int preferred = -1;
    int current = -1;
    int configured = -1;
    int best = -1;

    for (int i = modes.size() - 1; i >= 0; i--) {
        const drmModeModeInfo &m = modes.at(i);

        if (configuration == OutputConfigMode &&
                m.hdisplay == configurationSize.width() &&
                m.vdisplay == configurationSize.height()) {
            configured = i;
        }

        if (!memcmp(&crtc_mode, &m, sizeof m))
            current = i;

        if (m.type & DRM_MODE_TYPE_PREFERRED)
            preferred = i;

        best = i;
    }

    if (configuration == OutputConfigModeline) {
        modes << configurationModeline;
        configured = modes.size() - 1;
    }

    if (current < 0 && crtc_mode.clock != 0) {
        modes << crtc_mode;
        current = mode.size() - 1;
    }

    if (configuration == OutputConfigCurrent)
        configured = current;

    int selected_mode = -1;

    if (configured >= 0)
        selected_mode = configured;
    else if (preferred >= 0)
        selected_mode = preferred;
    else if (current >= 0)
        selected_mode = current;
    else if (best >= 0)
        selected_mode = best;

    if (selected_mode < 0) {
        qCWarning(lcKms, ) << "No modes available for output" << connectorName;
        return Q_NULLPTR;
    } else {
        int width = modes[selected_mode].hdisplay;
        int height = modes[selected_mode].vdisplay;
        int refresh = modes[selected_mode].vrefresh;
        qCDebug(lcKms) << "Selected mode" << selected_mode << ":" << width << "x" << height
                       << "@" << refresh << "hz for output" << connectorName;
    }

    static const int width = qEnvironmentVariableIntValue("GREENISLAND_QPA_PHYSICAL_WIDTH");
    static const int height = qEnvironmentVariableIntValue("GREENISLAND_QPA_PHYSICAL_HEIGHT");
    QSizeF size(width, height);
    if (size.isEmpty())
        size = QSizeF(connector->mmWidth, connector->mmHeight);

    EglFSKmsOutput output = {
        QString::fromUtf8(connectorName),
        connector->connector_id,
        crtc_id,
        size,
        selected_mode,
        false,
        drmModeGetCrtc(m_dri_fd, crtc_id),
        modes,
        connectorProperty(connector, QByteArrayLiteral("DPMS"))
    };

    m_crtc_allocator |= (1 << output.crtc_id);
    m_connector_allocator |= (1 << output.connector_id);

    return new EglFSKmsScreen(m_integration, this, output, pos);
}

drmModePropertyPtr EglFSKmsDevice::connectorProperty(drmModeConnectorPtr connector, const QByteArray &name)
{
    drmModePropertyPtr prop;

    for (int i = 0; i < connector->count_props; i++) {
        prop = drmModeGetProperty(m_dri_fd, connector->props[i]);
        if (!prop)
            continue;
        if (strcmp(prop->name, name.constData()) == 0)
            return prop;
        drmModeFreeProperty(prop);
    }

    return Q_NULLPTR;
}

void EglFSKmsDevice::pageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
    Q_UNUSED(fd);
    Q_UNUSED(sequence);
    Q_UNUSED(tv_sec);
    Q_UNUSED(tv_usec);

    EglFSKmsScreen *screen = static_cast<EglFSKmsScreen *>(user_data);
    screen->flipFinished();
}

EglFSKmsDevice::EglFSKmsDevice(EglFSKmsIntegration *integration, const QString &path)
    : m_integration(integration)
    , m_path(path)
    , m_dri_fd(-1)
    , m_gbm_device(Q_NULLPTR)
    , m_crtc_allocator(0)
    , m_connector_allocator(0)
    , m_globalCursor(Q_NULLPTR)
{
}

bool EglFSKmsDevice::open()
{
    Q_ASSERT(m_dri_fd == -1);
    Q_ASSERT(m_gbm_device == Q_NULLPTR);

    qCDebug(lcKms) << "Opening device" << m_path;
    m_dri_fd = qt_safe_open(m_path.toLocal8Bit().constData(), O_RDWR | O_CLOEXEC);
    if (m_dri_fd == -1) {
        qErrnoWarning("Could not open DRM device %s", qPrintable(m_path));
        return false;
    }

    qCDebug(lcKms) << "Creating GBM device for file descriptor" << m_dri_fd
                   << "obtained from" << m_path;
    m_gbm_device = gbm_create_device(m_dri_fd);
    if (!m_gbm_device) {
        qErrnoWarning("Could not create GBM device");
        qt_safe_close(m_dri_fd);
        m_dri_fd = -1;
        return false;
    }

    return true;
}

void EglFSKmsDevice::close()
{
    if (m_gbm_device) {
        gbm_device_destroy(m_gbm_device);
        m_gbm_device = Q_NULLPTR;
    }

    if (m_dri_fd != -1) {
        qt_safe_close(m_dri_fd);
        m_dri_fd = -1;
    }

    if (m_globalCursor)
        m_globalCursor->deleteLater();
    m_globalCursor = Q_NULLPTR;
}

void EglFSKmsDevice::createScreens()
{
    drmModeResPtr resources = drmModeGetResources(m_dri_fd);
    if (!resources) {
        qCWarning(lcKms, "drmModeGetResources failed");
        return;
    }

    EglFSKmsScreen *primaryScreen = Q_NULLPTR;
    QList<QPlatformScreen *> siblings;
    QPoint pos(0, 0);
    EglFSIntegration *integration = static_cast<EglFSIntegration *>(QGuiApplicationPrivate::platformIntegration());

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnectorPtr connector = drmModeGetConnector(m_dri_fd, resources->connectors[i]);
        if (!connector)
            continue;

        EglFSKmsScreen *screen = screenForConnector(resources, connector, pos);
        if (screen) {
            integration->addScreen(screen);
            pos.rx() += screen->geometry().width();
            siblings << screen;

            if (!primaryScreen)
                primaryScreen = screen;
        }

        drmModeFreeConnector(connector);
    }

    drmModeFreeResources(resources);

    if (!m_integration->separateScreens()) {
        Q_FOREACH (QPlatformScreen *screen, siblings)
            static_cast<EglFSKmsScreen *>(screen)->setVirtualSiblings(siblings);

        if (primaryScreen)
            m_globalCursor = new EglFSKmsCursor(primaryScreen);
    }
}

gbm_device *EglFSKmsDevice::device() const
{
    return m_gbm_device;
}

int EglFSKmsDevice::fd() const
{
    return m_dri_fd;
}

QPlatformCursor *EglFSKmsDevice::globalCursor() const
{
    return m_globalCursor;
}

void EglFSKmsDevice::handleDrmEvent()
{
    drmEventContext drmEvent = {
        DRM_EVENT_CONTEXT_VERSION,
        Q_NULLPTR,      // vblank handler
        pageFlipHandler // page flip handler
    };

    drmHandleEvent(m_dri_fd, &drmEvent);
}

} // namespace Platform

} // namespace GreenIsland
