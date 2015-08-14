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

#include <QtCore/QUrl>

#include "abstractcompositor.h"
#include "clientconnection.h"

#include "protocols/qt/qtwindowmanager.h"

#include "wayland_wrapper/qwldisplay_p.h"
#include "wayland_wrapper/qwlcompositor_p.h"

#include <wayland-server.h>

namespace GreenIsland {

QtWindowManager::QtWindowManager(AbstractCompositor *compositor, QObject *parent)
    : QObject(parent)
    , QtWaylandServer::qt_windowmanager()
    , m_showIsFullScreen(false)
    , m_compositor(compositor)
{
}

QtWindowManager::~QtWindowManager()
{
}

void QtWindowManager::initialize(GreenIsland::Display *waylandDisplay)
{
    init(waylandDisplay->handle(), 1);
}

void QtWindowManager::setShowIsFullScreen(bool value)
{
    m_showIsFullScreen = value;

    Q_FOREACH (Resource *resource, resourceMap().values())
        send_hints(resource->handle, static_cast<int32_t>(m_showIsFullScreen));
}

void QtWindowManager::sendQuitMessage(wl_client *client)
{
    Resource *resource = resourceMap().value(client);

    if (resource)
        send_quit(resource->handle);
}

void QtWindowManager::windowmanager_bind_resource(Resource *resource)
{
    send_hints(resource->handle, static_cast<int32_t>(m_showIsFullScreen));
}

void QtWindowManager::windowmanager_destroy_resource(Resource *resource)
{
    m_urls.remove(resource);
}

void QtWindowManager::windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &newUrl)
{
    QString url = m_urls.value(resource, QString());

    url.append(newUrl);

    if (remaining)
        m_urls.insert(resource, url);
    else {
        m_urls.remove(resource);
        m_compositor->openUrl(ClientConnection::fromWlClient(resource->client()), QUrl(url));
    }
}

} // namespace GreenIsland
