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

#include <QtCore/QLoggingCategory>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformwindow.h>

#include <GreenIsland/Platform/EglFSIntegration>
#include <GreenIsland/Platform/EglFSWindow>
#include <GreenIsland/Platform/VtHandler>

#include "eglfskmsscreen.h"
#include "eglfskmsdevice.h"
#include "eglfskmscursor.h"

namespace GreenIsland {

namespace Platform {

Q_DECLARE_LOGGING_CATEGORY(lcKms)

class EglFSKmsInterruptHandler : public QObject
{
public:
    EglFSKmsInterruptHandler(EglFSKmsScreen *screen) : m_screen(screen) {
        m_vt = static_cast<EglFSIntegration *>(QGuiApplicationPrivate::platformIntegration())->vtHandler();
        connect(m_vt, &VtHandler::interrupted, this, &EglFSKmsInterruptHandler::restoreVideoMode);
        connect(m_vt, &VtHandler::aboutToSuspend, this, &EglFSKmsInterruptHandler::restoreVideoMode);
        connect(m_vt, &VtHandler::activeChanged, this, &EglFSKmsInterruptHandler::activeChanged);
    }

public Q_SLOTS:
    void restoreVideoMode() { m_screen->restoreMode(); }
    void activeChanged(bool active)
    {
        if (active)
            drmSetMaster(m_screen->device()->fd());
        else
            drmDropMaster(m_screen->device()->fd());
    }

private:
    VtHandler *m_vt;
    EglFSKmsScreen *m_screen;
};

void EglFSKmsScreen::bufferDestroyedHandler(gbm_bo *bo, void *data)
{
    FrameBuffer *fb = static_cast<FrameBuffer *>(data);

    if (fb->fb) {
        gbm_device *device = gbm_bo_get_device(bo);
        drmModeRmFB(gbm_device_get_fd(device), fb->fb);
    }

    delete fb;
}

EglFSKmsScreen::FrameBuffer *EglFSKmsScreen::framebufferForBufferObject(gbm_bo *bo)
{
    {
        FrameBuffer *fb = static_cast<FrameBuffer *>(gbm_bo_get_user_data(bo));
        if (fb)
            return fb;
    }

    uint32_t width = gbm_bo_get_width(bo);
    uint32_t height = gbm_bo_get_height(bo);
    uint32_t stride = gbm_bo_get_stride(bo);
    uint32_t handle = gbm_bo_get_handle(bo).u32;

    QScopedPointer<FrameBuffer> fb(new FrameBuffer);

    int ret = drmModeAddFB(m_device->fd(), width, height, 24, 32,
                           stride, handle, &fb->fb);

    if (ret) {
        qCWarning(lcKms, "Failed to create KMS FB!");
        return Q_NULLPTR;
    }

    gbm_bo_set_user_data(bo, fb.data(), bufferDestroyedHandler);
    return fb.take();
}

EglFSKmsScreen::EglFSKmsScreen(EglFSKmsIntegration *integration,
                               EglFSKmsDevice *device,
                               EglFSKmsOutput output,
                               QPoint position)
    : EglFSScreen(eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(device->device())))
    , m_integration(integration)
    , m_device(device)
    , m_gbm_surface(Q_NULLPTR)
    , m_gbm_bo_current(Q_NULLPTR)
    , m_gbm_bo_next(Q_NULLPTR)
    , m_pendingMode(-1)
    , m_output(output)
    , m_pos(position)
    , m_cursor(Q_NULLPTR)
    , m_powerState(PowerStateOn)
    , m_interruptHandler(new EglFSKmsInterruptHandler(this))
{
    m_siblings << this;

    if (m_output.edid_blob) {
        if (parseEdid(m_edid))
            qCDebug(lcKms, "EDID data for output \"%s\": identifier '%s', manufacturer '%s', model '%s', serial '%s', physical size: %.2fx%.2f",
                    name().toLatin1().constData(),
                    m_edid.identifier.toLatin1().constData(),
                    m_edid.manufacturer.toLatin1().constData(),
                    m_edid.model.toLatin1().constData(),
                    m_edid.serialNumber.toLatin1().constData(),
                    m_edid.physicalSize.width(), m_edid.physicalSize.height());
        else
            qCWarning(lcKms) << "Failed to parse EDID data for output" << name();
    } else {
        qCWarning(lcKms) << "No EDID data for output" << name();
    }
}

EglFSKmsScreen::~EglFSKmsScreen()
{
    if (m_output.dpms_prop) {
        drmModeFreeProperty(m_output.dpms_prop);
        m_output.dpms_prop = Q_NULLPTR;
    }
    if (m_output.edid_blob) {
        drmModeFreePropertyBlob(m_output.edid_blob);
        m_output.edid_blob = Q_NULLPTR;
    }
    restoreMode();
    if (m_output.saved_crtc) {
        drmModeFreeCrtc(m_output.saved_crtc);
        m_output.saved_crtc = Q_NULLPTR;
    }
    delete m_interruptHandler;
}

QRect EglFSKmsScreen::geometry() const
{
    const int mode = m_output.mode;
    return QRect(m_pos.x(), m_pos.y(),
                 m_output.modes[mode].hdisplay,
                 m_output.modes[mode].vdisplay);
}

int EglFSKmsScreen::depth() const
{
    return 32;
}

QImage::Format EglFSKmsScreen::format() const
{
    return QImage::Format_RGB32;
}

QSizeF EglFSKmsScreen::physicalSize() const
{
    // Some connectors report empty physical size, this happens especially
    // on virtual machines resulting in NaN DPI breaking font rendering,
    // icons and other things on the other end EDID often reports the wrong
    // physical size so what we do here is pick up the physical size reported
    // by the connector or, if empty, from EDID.
    // If neither is valid we calculate a physical size for 100 DPI.
    QSizeF size = m_output.physical_size.isEmpty() ? m_edid.physicalSize : m_output.physical_size;
    if (Q_UNLIKELY(size.isEmpty())) {
        // pixelsPerMm is 25.4 (mm per inch) divided by 100 (default physical DPI)
        const qreal pixelsPerMm = 0.254;
        size = QSizeF(geometry().size().width() * pixelsPerMm, geometry().size().height() * pixelsPerMm);
    }
    return size;
}

QDpi EglFSKmsScreen::logicalDpi() const
{
    const QSizeF ps = physicalSize();
    const QSize s = geometry().size();

    if (!ps.isEmpty() && !s.isEmpty())
        return QDpi(25.4 * s.width() / ps.width(),
                    25.4 * s.height() / ps.height());
    else
        return QDpi(100, 100);
}

Qt::ScreenOrientation EglFSKmsScreen::nativeOrientation() const
{
    return Qt::PrimaryOrientation;
}

Qt::ScreenOrientation EglFSKmsScreen::orientation() const
{
    return Qt::PrimaryOrientation;
}

QString EglFSKmsScreen::name() const
{
    return m_output.name;
}

QPlatformCursor *EglFSKmsScreen::cursor() const
{
    if (m_integration->hwCursor()) {
        if (!m_integration->separateScreens())
            return m_device->globalCursor();

        if (m_cursor.isNull()) {
            EglFSKmsScreen *that = const_cast<EglFSKmsScreen *>(this);
            that->m_cursor.reset(new EglFSKmsCursor(that));
        }

        return m_cursor.data();
    } else {
        return EglFSScreen::cursor();
    }
}

gbm_surface *EglFSKmsScreen::createSurface()
{
    if (!m_gbm_surface) {
        qCDebug(lcKms, "Creating window for screen \"%s\" with size %dx%d",
                name().toUtf8().constData(),
                geometry().size().width(),
                geometry().size().height());
        m_gbm_surface = gbm_surface_create(m_device->device(),
                                           geometry().width(),
                                           geometry().height(),
                                           GBM_FORMAT_XRGB8888,
                                           GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    }
    return m_gbm_surface;
}

void EglFSKmsScreen::destroySurface()
{
    if (m_gbm_bo_current) {
        gbm_bo_destroy(m_gbm_bo_current);
        m_gbm_bo_current = Q_NULLPTR;
    }

    if (m_gbm_bo_next) {
        gbm_bo_destroy(m_gbm_bo_next);
        m_gbm_bo_next = Q_NULLPTR;
    }

    if (m_gbm_surface) {
        gbm_surface_destroy(m_gbm_surface);
        m_gbm_surface = Q_NULLPTR;
    }
}

void EglFSKmsScreen::waitForFlip()
{
    // Don't lock the mutex unless we actually need to
    if (!m_gbm_bo_next)
        return;

    QMutexLocker lock(&m_waitForFlipMutex);
    while (m_gbm_bo_next)
        m_device->handleDrmEvent();
}

void EglFSKmsScreen::flip()
{
    if (!m_gbm_surface) {
        qCWarning(lcKms, "Cannot sync before platform init!");
        return;
    }

    m_gbm_bo_next = gbm_surface_lock_front_buffer(m_gbm_surface);
    if (!m_gbm_bo_next) {
        qCWarning(lcKms, "Could not lock GBM surface front buffer!");
        return;
    }

    FrameBuffer *fb = framebufferForBufferObject(m_gbm_bo_next);

    if (!m_output.mode_set) {
        qCDebug(lcKms, "Changing mode to %d: %dx%d @ %.2f Hz",
                m_output.mode,
                geometry().size().width(),
                geometry().size().height(),
                refreshRate());

        int ret = drmModeSetCrtc(m_device->fd(),
                                 m_output.crtc_id,
                                 fb->fb,
                                 0, 0,
                                 &m_output.connector_id, 1,
                                 &m_output.modes[m_output.mode]);

        if (ret) {
            qErrnoWarning("Could not set DRM mode!");
        } else {
            m_output.mode_set = true;
            setPowerState(PowerStateOn);
        }
    }

    int ret = drmModePageFlip(m_device->fd(),
                              m_output.crtc_id,
                              fb->fb,
                              DRM_MODE_PAGE_FLIP_EVENT,
                              this);
    if (ret) {
        qErrnoWarning("Could not queue DRM page flip!");
        gbm_surface_release_buffer(m_gbm_surface, m_gbm_bo_next);
        m_gbm_bo_next = Q_NULLPTR;
    }
}

void EglFSKmsScreen::flipFinished()
{
    if (m_gbm_bo_current)
        gbm_surface_release_buffer(m_gbm_surface,
                                   m_gbm_bo_current);

    m_gbm_bo_current = m_gbm_bo_next;
    m_gbm_bo_next = Q_NULLPTR;
}

void EglFSKmsScreen::resizeSurface()
{
    m_output.mode = m_pendingMode;
    m_pendingMode = -1;
    m_output.mode_set = false;

    destroySurface();
    createSurface();

    QWindowSystemInterface::handleScreenGeometryChange(screen(), geometry(), availableGeometry());
    QWindowSystemInterface::handleScreenRefreshRateChange(screen(), refreshRate());
    QWindowSystemInterface::handleScreenLogicalDotsPerInchChange(screen(), logicalDpi().first, logicalDpi().second);

    resizeMaximizedWindows();
}

void EglFSKmsScreen::restoreMode()
{
    if (m_output.mode_set && m_output.saved_crtc) {
        drmModeSetCrtc(m_device->fd(),
                       m_output.saved_crtc->crtc_id,
                       m_output.saved_crtc->buffer_id,
                       0, 0,
                       &m_output.connector_id, 1,
                       &m_output.saved_crtc->mode);

        m_output.mode_set = false;
    }
}

qreal EglFSKmsScreen::refreshRate() const
{
    quint32 refresh = m_output.modes[m_output.mode].vrefresh;
    return refresh > 0 ? refresh : 60;
}

EglFSScreen::PowerState EglFSKmsScreen::powerState() const
{
    return m_powerState;
}

void EglFSKmsScreen::setPowerState(EglFSScreen::PowerState state)
{
    if (!m_output.dpms_prop)
        return;

    drmModeConnectorSetProperty(m_device->fd(), m_output.connector_id,
                                m_output.dpms_prop->prop_id, (int)state);
    m_powerState = state;
}

QList<EglFSScreen::Mode> EglFSKmsScreen::modes() const
{
    QList<EglFSScreen::Mode> list;

    Q_FOREACH (const drmModeModeInfo &info, m_output.modes)
        list.append({QSize(info.hdisplay, info.vdisplay),
                     qreal(info.vrefresh > 0 ? info.vrefresh : 60)});

    return list;
}

int EglFSKmsScreen::currentMode() const
{
    return m_output.mode;
}

void EglFSKmsScreen::setCurrentMode(int modeId)
{
    if (modeId < 0 || modeId >= m_output.modes.size()) {
        qCWarning(lcKms, "Invalid mode passed to EglFSKmsScreen::setCurrentMode()");
        return;
    }

    if (m_pendingMode >= 0) {
        qCWarning(lcKms, "Cannot change mode when another mode set is pending");
        return;
    }

    m_pendingMode = modeId;
}

int EglFSKmsScreen::preferredMode() const
{
    return m_output.preferred_mode;
}

void EglFSKmsScreen::setPreferredMode(int modeId)
{
    if (modeId < 0 || modeId >= m_output.modes.size()) {
        qCWarning(lcKms, "Invalid mode passed to EglFSKmsScreen::setPreferredMode()");
        return;
    }

    m_output.preferred_mode = modeId;
}

QString EglFSKmsScreen::identifier() const
{
    return m_edid.identifier;
}

QString EglFSKmsScreen::manufacturer() const
{
    return m_edid.manufacturer;
}

QString EglFSKmsScreen::model() const
{
    return m_edid.model;
}

QString EglFSKmsScreen::serialNumber() const
{
    return m_edid.serialNumber;
}

bool EglFSKmsScreen::parseEdid(EglFSKmsEdid &edid)
{
    const quint8 *data = reinterpret_cast<quint8 *>(m_output.edid_blob->data);
    const size_t length = m_output.edid_blob->length;

    // Verify header
    if (length < 128)
        return false;
    if (data[0] != 0x00 || data[1] != 0xff)
        return false;

    /* Decode the PNP ID from three 5 bit words packed into 2 bytes
     * /--08--\/--09--\
     * 7654321076543210
     * |\---/\---/\---/
     * R  C1   C2   C3 */
    char id[4];
    id[0] = 'A' + ((data[EdidOffsetPnpId] & 0x7c) / 4) - 1;
    id[1] = 'A' + ((data[EdidOffsetPnpId] & 0x3) * 8) + ((data[EdidOffsetPnpId + 1] & 0xe0) / 32) - 1;
    id[2] = 'A' + (data[EdidOffsetPnpId + 1] & 0x1f) - 1;
    id[3] = '\0';
    edid.manufacturer = QString::fromLatin1(id, 4);

    // Serial number, will be overwritten by an ASCII descriptor
    // when and if it will be found
    quint32 serial = data[EdidOffsetSerial]
            + (data[EdidOffsetSerial + 1] << 8)
            + (data[EdidOffsetSerial + 2] << 16)
            + (data[EdidOffsetSerial + 3] << 24);
    if (serial > 0)
        edid.serialNumber = QString::number(serial);

    // Parse EDID data
    for (int i = 0; i < 5; ++i) {
        const uint offset = EdidOffsetDataBlocks + i * 18;

        if (data[offset] != 0 || data[offset + 1] != 0 || data[offset + 2] != 0)
            continue;

        if (data[offset + 3] == EdidDescriptorDisplayProductName)
            edid.model = parseEdidString(&data[offset + 5]);
        else if (data[offset + 3] == EdidDescriptorAlphanumericDataString)
            edid.identifier = parseEdidString(&data[offset + 5]);
        else if (data[offset + 3] == EdidDescriptorDisplayProductSerialNumber)
            edid.serialNumber = parseEdidString(&data[offset + 5]);
    }

    // PNP ID is the same as EISA ID
    if (edid.identifier.isEmpty())
        edid.identifier = edid.manufacturer;

    // Physical size
    edid.physicalSize = QSizeF(data[EdidOffsetPhysicalWidth], data[EdidOffsetPhysicalHeight]) * 10;

    return true;
}

QString EglFSKmsScreen::parseEdidString(const quint8 *data)
{
    QByteArray buffer(reinterpret_cast<const char *>(data), 12);

    // Erase carriage return and line feed
    buffer = buffer.replace('\r', '\0').replace('\n', '\0');

    // Replace non-printable characters with dash
    for (int i = 0; i < buffer.count(); ++i) {
        if (buffer[i] < '\040' && buffer[i] > '\176')
            buffer[i] = '-';
    }

    return QString::fromLatin1(buffer.trimmed());
}

} // namespace Platform

} // namespace GreenIsland
