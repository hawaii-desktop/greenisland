/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WLDATASOURCE_H
#define WLDATASOURCE_H

#include "qwayland-server-wayland.h"
#include <QObject>
#include <QtCore/QList>


namespace GreenIsland {

class WlDataOffer;
class WlDataDevice;
class WlDataDeviceManager;

class WlDataSource : public QObject, public QtWaylandServer::wl_data_source
{
public:
    WlDataSource(struct wl_client *client, uint32_t id, uint32_t time);
    ~WlDataSource();
    uint32_t time() const;
    QList<QString> mimeTypes() const;

    void accept(const QString &mimeType);
    void send(const QString &mimeType,int fd);
    void cancel();

    void setManager(WlDataDeviceManager *mgr);
    void setDevice(WlDataDevice *device);

    static WlDataSource *fromResource(struct ::wl_resource *resource);

protected:
    void data_source_offer(Resource *resource, const QString &mime_type) Q_DECL_OVERRIDE;
    void data_source_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void data_source_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

private:
    uint32_t m_time;
    QList<QString> m_mimeTypes;

    WlDataDevice *m_device;
    WlDataDeviceManager *m_manager;
};

}


#endif // WLDATASOURCE_H
