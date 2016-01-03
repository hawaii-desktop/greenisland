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

#ifndef GREENISLAND_EGLFSKMSDEVICE_H
#define GREENISLAND_EGLFSKMSDEVICE_H

#include "eglfskmscursor.h"
#include "eglfskmsintegration.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

namespace GreenIsland {

namespace Platform {

class EglFSKmsScreen;

class EglFSKmsDevice
{
public:
    EglFSKmsDevice(EglFSKmsIntegration *integration, const QString &path);

    bool open();
    void close();

    void createScreens();

    gbm_device *device() const;
    int fd() const;

    QPlatformCursor *globalCursor() const;

    void handleDrmEvent();

private:
    Q_DISABLE_COPY(EglFSKmsDevice)

    EglFSKmsIntegration *m_integration;
    QString m_path;
    int m_dri_fd;
    gbm_device *m_gbm_device;

    quint32 m_crtc_allocator;
    quint32 m_connector_allocator;

    EglFSKmsCursor *m_globalCursor;

    int crtcForConnector(drmModeResPtr resources, drmModeConnectorPtr connector);
    EglFSKmsScreen *screenForConnector(drmModeResPtr resources, drmModeConnectorPtr connector, QPoint pos);
    drmModePropertyPtr connectorProperty(drmModeConnectorPtr connector, const QByteArray &name);

    static void pageFlipHandler(int fd,
                                unsigned int sequence,
                                unsigned int tv_sec,
                                unsigned int tv_usec,
                                void *user_data);
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSKMSDEVICE
