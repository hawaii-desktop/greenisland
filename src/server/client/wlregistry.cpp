/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
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

#include "wlregistry.h"
#include "wlshmpool.h"
#include "qwayland-fullscreen-shell.h"

#include <wayland-client.h>
#include <wayland-server.h>

Q_LOGGING_CATEGORY(WLREGISTRY, "greenisland.wlregistry")

namespace GreenIsland {

/*
 * WlRegistryPrivate
 */

static WlRegistry::Interface nameToInterface(const char *interface)
{
    if (strcmp(interface, "wl_compositor") == 0)
        return WlRegistry::Compositor;
    else if (strcmp(interface, "wl_seat") == 0)
        return WlRegistry::Seat;
    else if (strcmp(interface, "wl_shm") == 0)
        return WlRegistry::Shm;
    else if (strcmp(interface, "_wl_fullscreen_shell") == 0)
        return WlRegistry::FullscreenShell;
    return WlRegistry::Unknown;
}

static const wl_interface *wlInterface(WlRegistry::Interface interface)
{
    switch (interface) {
    case WlRegistry::Compositor:
        return &wl_compositor_interface;
    case WlRegistry::Seat:
        return &wl_seat_interface;
    case WlRegistry::Shm:
        return &wl_shm_interface;
    case WlRegistry::FullscreenShell:
        return &_wl_fullscreen_shell_interface;
    default:
        break;
    }

    return Q_NULLPTR;
}

class WlRegistryPrivate
{
public:
    WlRegistryPrivate(WlRegistry *q)
        : registry(Q_NULLPTR)
        , callback(Q_NULLPTR)
        , q(q)
    {
    }

    ~WlRegistryPrivate()
    {
        if (registry)
            wl_registry_destroy(registry);
        if (callback)
            wl_callback_destroy(callback);
    }

    void setup()
    {
        wl_registry_add_listener(registry, &s_registryListener, this);
        wl_callback_add_listener(callback, &s_callbackListener, this);
    }

    template <typename T>
    T *bind(WlRegistry::Interface interface)
    {
        QList<InterfaceInfo>::iterator it;
        for (it = m_interfaces.begin(); it != m_interfaces.end(); ++it) {
            InterfaceInfo info = *it;

            if (info.interface == interface) {
                auto t = reinterpret_cast<T*>(wl_registry_bind(registry, info.name,
                                                               wlInterface(interface), info.version));
                return t;
            }
        }

        qCWarning(WLREGISTRY) << "Cannot bind unknown interface";
        return Q_NULLPTR;
    }

    wl_registry *registry;
    wl_callback *callback;

private:
    WlRegistry *q;

    struct InterfaceInfo {
        WlRegistry::Interface interface;
        quint32 name;
        quint32 version;
    };
    QList<InterfaceInfo> m_interfaces;

    static const struct wl_registry_listener s_registryListener;
    static const struct wl_callback_listener s_callbackListener;

    void handleAnnounce(const char *interface, quint32 name, quint32 version)
    {
        Q_EMIT q->interfaceAnnounced(QString::fromUtf8(interface), name, version);

        WlRegistry::Interface i = nameToInterface(interface);
        m_interfaces.append({i, name, version});

        switch (i) {
        case WlRegistry::Compositor:
            Q_EMIT q->compositorAnnounced(name, version);
            break;
        case WlRegistry::Seat:
            Q_EMIT q->seatAnnounced(name, version);
            break;
        case WlRegistry::Shm:
            Q_EMIT q->shmAnnounced(name, version);
            break;
        case WlRegistry::FullscreenShell:
            Q_EMIT q->fullscreenShellAnnounced(name, version);
            break;
        default:
            break;
        }
    }

    void handleRemove(quint32 name)
    {
        QList<InterfaceInfo>::iterator it;
        for (it = m_interfaces.begin(); it != m_interfaces.end(); ++it) {
            InterfaceInfo info = *it;
            if (info.name == name) {
                m_interfaces.erase(it);

                switch (info.interface) {
                case WlRegistry::Compositor:
                    Q_EMIT q->compositorRemoved(name);
                    break;
                case WlRegistry::Seat:
                    Q_EMIT q->seatRemoved(name);
                    break;
                case WlRegistry::Shm:
                    Q_EMIT q->shmRemoved(name);
                    break;
                case WlRegistry::FullscreenShell:
                    Q_EMIT q->fullscreenShellRemoved(name);
                    break;
                default:
                    break;
                }

                break;
            }
        }

        Q_EMIT q->interfaceRemoved(name);

        if (m_interfaces.size() == 0)
            Q_EMIT q->interfacesRemoved();
    }

    void handleSync()
    {
        Q_EMIT q->interfacesAnnounced();
    }

    static void globalAnnounce(void *data, wl_registry *registry, uint32_t name,
                               const char *interface, uint32_t version)
    {
        auto self = reinterpret_cast<WlRegistryPrivate *>(data);
        Q_ASSERT(registry == self->registry);
        self->handleAnnounce(interface, name, version);
    }

    static void globalRemove(void *data, wl_registry *registry, uint32_t name)
    {
        auto self = reinterpret_cast<WlRegistryPrivate *>(data);
        Q_ASSERT(registry == self->registry);
        self->handleRemove(name);
    }

    static void globalSync(void *data, wl_callback *callback, uint32_t serial)
    {
        Q_UNUSED(serial)

        auto self = reinterpret_cast<WlRegistryPrivate *>(data);
        Q_ASSERT(callback == self->callback);
        self->handleSync();
        wl_callback_destroy(self->callback);
        self->callback = Q_NULLPTR;
    }
};

const struct wl_registry_listener WlRegistryPrivate::s_registryListener = {
    globalAnnounce,
    globalRemove
};

const struct wl_callback_listener WlRegistryPrivate::s_callbackListener = {
   globalSync
};


/*
 * WlRegistry
 */

WlRegistry::WlRegistry(QObject *parent)
    : QObject(parent)
    , d_ptr(new WlRegistryPrivate(this))
{
}

WlRegistry::~WlRegistry()
{
    delete d_ptr;
}

bool WlRegistry::isValid() const
{
    Q_D(const WlRegistry);
    return d->registry != Q_NULLPTR;
}

wl_registry *WlRegistry::registry() const
{
    Q_D(const WlRegistry);
    return d->registry;
}

void WlRegistry::create(wl_display *display)
{
    Q_D(WlRegistry);

    Q_ASSERT(display);
    Q_ASSERT(!isValid());
    d->registry = wl_display_get_registry(display);
    d->callback = wl_display_sync(display);
}

void WlRegistry::setup()
{
    Q_D(WlRegistry);
    Q_ASSERT(isValid());
    d->setup();
}

wl_compositor *WlRegistry::bindCompositor()
{
    Q_D(WlRegistry);
    return d->bind<wl_compositor>(Compositor);
}

WlShmPool *WlRegistry::createShmPool(QObject *parent)
{
    Q_D(WlRegistry);
    return new WlShmPool(d->bind<wl_shm>(Shm), parent);
}

}

#include "moc_wlregistry.cpp"
