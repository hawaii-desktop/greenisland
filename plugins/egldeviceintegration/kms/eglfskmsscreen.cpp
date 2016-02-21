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
#include <QtGui/private/qguiapplication_p.h>

#include <GreenIsland/Platform/EglFSIntegration>
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
    }

public slots:
    void restoreVideoMode() { m_screen->restoreMode(); }

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
    , m_output(output)
    , m_pos(position)
    , m_cursor(Q_NULLPTR)
    , m_powerState(PowerStateOn)
    , m_interruptHandler(new EglFSKmsInterruptHandler(this))
{
    m_siblings << this;
}

EglFSKmsScreen::~EglFSKmsScreen()
{
    if (m_output.dpms_prop) {
        drmModeFreeProperty(m_output.dpms_prop);
        m_output.dpms_prop = Q_NULLPTR;
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
    // icons and other things: calculate a physical size for 100 DPI
    QSizeF size = m_output.physical_size;
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
        qCDebug(lcKms) << "Creating window for screen" << name();
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

} // namespace Platform

} // namespace GreenIsland
