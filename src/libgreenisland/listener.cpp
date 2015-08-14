/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "listener.h"

#include <wayland-server.h>

namespace GreenIsland {

/*
 * ListenerPrivate
 */

class ListenerPrivate
{
public:
    ListenerPrivate(Listener *self)
    {
        wrapper.parent = self;
        wrapper.listener.notify = handler;
        wl_list_init(&wrapper.listener.link);
    }

    struct Wrapper {
        wl_listener listener;
        Listener *parent;
    };
    Wrapper wrapper;

private:
    static void handler(wl_listener *listener, void *data)
    {
        Q_UNUSED(data);

        Listener *parent = reinterpret_cast<Wrapper *>(listener)->parent;
        Q_EMIT parent->triggered();
    }
};

/*
 * Listener
 */

Listener::Listener(QObject *parent)
    : QObject(parent)
    , d_ptr(new ListenerPrivate(this))
{
}

Listener::~Listener()
{
    delete d_ptr;
}

void Listener::listenForDestruction(wl_resource *resource)
{
    Q_D(Listener);

    wl_resource_add_destroy_listener(resource, &d->wrapper.listener);
}

void Listener::reset()
{
    Q_D(Listener);

    wl_list_remove(&d->wrapper.listener.link);
    wl_list_init(&d->wrapper.listener.link);
}

} // namespace GreenIsland

#include "moc_listener.cpp"
