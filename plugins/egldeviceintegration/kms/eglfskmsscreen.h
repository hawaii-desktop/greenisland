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

#ifndef GREENISLAND_EGLFSKMSSCREEN_H
#define GREENISLAND_EGLFSKMSSCREEN_H

#include <QtCore/QList>
#include <QtCore/QMutex>

#include <GreenIsland/Platform/EglFSScreen>

#include "eglfskmsintegration.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

namespace GreenIsland {

namespace Platform {

class EglFSKmsDevice;
class EglFSKmsCursor;
class EglFSKmsInterruptHandler;

struct EglFSKmsOutput
{
    QString name;
    uint32_t connector_id;
    uint32_t crtc_id;
    QSizeF physical_size;
    int mode; // index of selected mode in list below
    bool mode_set;
    drmModeCrtcPtr saved_crtc;
    QList<drmModeModeInfo> modes;
    drmModePropertyPtr dpms_prop;
};

class EglFSKmsScreen : public EglFSScreen
{
public:
    EglFSKmsScreen(EglFSKmsIntegration *integration,
                   EglFSKmsDevice *device,
                   EglFSKmsOutput output,
                   QPoint position);
    ~EglFSKmsScreen();

    QRect geometry() const Q_DECL_OVERRIDE;
    int depth() const Q_DECL_OVERRIDE;
    QImage::Format format() const Q_DECL_OVERRIDE;

    QSizeF physicalSize() const Q_DECL_OVERRIDE;
    QDpi logicalDpi() const Q_DECL_OVERRIDE;
    Qt::ScreenOrientation nativeOrientation() const Q_DECL_OVERRIDE;
    Qt::ScreenOrientation orientation() const Q_DECL_OVERRIDE;

    QString name() const Q_DECL_OVERRIDE;

    QPlatformCursor *cursor() const Q_DECL_OVERRIDE;

    qreal refreshRate() const Q_DECL_OVERRIDE;

    QList<QPlatformScreen *> virtualSiblings() const Q_DECL_OVERRIDE { return m_siblings; }
    void setVirtualSiblings(QList<QPlatformScreen *> sl) { m_siblings = sl; }

    EglFSKmsIntegration *integration() const { return m_integration; }
    EglFSKmsDevice *device() const { return m_device; }

    gbm_surface *surface() const { return m_gbm_surface; }
    gbm_surface *createSurface();
    void destroySurface();

    void waitForFlip();
    void flip();
    void flipFinished();

    EglFSKmsOutput &output() { return m_output; }
    void restoreMode();

    EglFSScreen::PowerState powerState() const Q_DECL_OVERRIDE;
    void setPowerState(EglFSScreen::PowerState state) Q_DECL_OVERRIDE;

private:
    EglFSKmsIntegration *m_integration;
    EglFSKmsDevice *m_device;
    gbm_surface *m_gbm_surface;

    gbm_bo *m_gbm_bo_current;
    gbm_bo *m_gbm_bo_next;

    EglFSKmsOutput m_output;
    QPoint m_pos;
    QScopedPointer<EglFSKmsCursor> m_cursor;

    QList<QPlatformScreen *> m_siblings;

    PowerState m_powerState;

    struct FrameBuffer {
        FrameBuffer() : fb(0) {}
        uint32_t fb;
    };
    static void bufferDestroyedHandler(gbm_bo *bo, void *data);
    FrameBuffer *framebufferForBufferObject(gbm_bo *bo);

    static QMutex m_waitForFlipMutex;

    EglFSKmsInterruptHandler *m_interruptHandler;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSKMSSCREEN_H
