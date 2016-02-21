/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include "compositor.h"
#include "compositor_p.h"
#include "fullscreenshell.h"
#include "fullscreenshell_p.h"
#include "output.h"
#include "output_p.h"
#include "registry.h"
#include "registry_p.h"
#include "screencaster.h"
#include "screencaster_p.h"
#include "screenshooter.h"
#include "screenshooter_p.h"
#include "seat.h"
#include "seat_p.h"
#include "shm.h"
#include "shm_p.h"
#include "qwayland-fullscreen-shell.h"

#include <wayland-client.h>
#include <wayland-server.h>

Q_LOGGING_CATEGORY(WLREGISTRY, "greenisland.client.registry")

namespace GreenIsland {

namespace Client {

/*
 * RegistryPrivate
 */

static Registry::Interface nameToInterface(const char *interface)
{
    if (strcmp(interface, "wl_compositor") == 0)
        return Registry::CompositorInterface;
    else if (strcmp(interface, "_wl_fullscreen_shell") == 0)
        return Registry::FullscreenShellInterface;
    else if (strcmp(interface, "wl_output") == 0)
        return Registry::OutputInterface;
    else if (QByteArray(interface) == Screencaster::interfaceName())
        return Registry::ScreencasterInterface;
    else if (QByteArray(interface) == Screenshooter::interfaceName())
        return Registry::ScreenshooterInterface;
    else if (strcmp(interface, "wl_seat") == 0)
        return Registry::SeatInterface;
    else if (strcmp(interface, "wl_shm") == 0)
        return Registry::ShmInterface;
    return Registry::UnknownInterface;
}

static const wl_interface *wlInterface(Registry::Interface interface)
{
    switch (interface) {
    case Registry::CompositorInterface:
        return &wl_compositor_interface;
    case Registry::OutputInterface:
        return &wl_output_interface;
    case Registry::FullscreenShellInterface:
        return &_wl_fullscreen_shell_interface;
    case Registry::ScreencasterInterface:
        return &greenisland_screencaster_interface;
    case Registry::ScreenshooterInterface:
        return &greenisland_screenshooter_interface;
    case Registry::SeatInterface:
        return &wl_seat_interface;
    case Registry::ShmInterface:
        return &wl_shm_interface;
    default:
        break;
    }

    return Q_NULLPTR;
}

RegistryPrivate::RegistryPrivate()
    : display(Q_NULLPTR)
    , registry(Q_NULLPTR)
    , callback(Q_NULLPTR)
{
}

RegistryPrivate::~RegistryPrivate()
{
    if (registry)
        wl_registry_destroy(registry);
    if (callback)
        wl_callback_destroy(callback);
}

void RegistryPrivate::setup()
{
    wl_registry_add_listener(registry, &s_registryListener, this);
    wl_callback_add_listener(callback, &s_callbackListener, this);
}

template <typename T>
T *RegistryPrivate::bind(Registry::Interface interface)
{
    QList<InterfaceInfo>::iterator it;
    for (it = interfaces.begin(); it != interfaces.end(); ++it) {
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

void RegistryPrivate::handleAnnounce(const char *interface, quint32 name, quint32 version)
{
    Q_Q(Registry);

    Q_EMIT q->interfaceAnnounced(QByteArray(interface), name, version);

    Registry::Interface i = nameToInterface(interface);
    interfaces.append({i, name, version});

    switch (i) {
    case Registry::CompositorInterface:
        Q_EMIT q->compositorAnnounced(name, version);
        break;
    case Registry::FullscreenShellInterface:
        Q_EMIT q->fullscreenShellAnnounced(name, version);
        break;
    case Registry::OutputInterface:
        Q_EMIT q->outputAnnounced(name, version);
        break;
    case Registry::ScreencasterInterface:
        Q_EMIT q->screencasterAnnounced(name, version);
        break;
    case Registry::ScreenshooterInterface:
        Q_EMIT q->screenshooterAnnounced(name, version);
        break;
    case Registry::SeatInterface:
        Q_EMIT q->seatAnnounced(name, version);
        break;
    case Registry::ShmInterface:
        Q_EMIT q->shmAnnounced(name, version);
        break;
    default:
        break;
    }
}

void RegistryPrivate::handleRemove(quint32 name)
{
    Q_Q(Registry);

    QList<InterfaceInfo>::iterator it;
    for (it = interfaces.begin(); it != interfaces.end(); ++it) {
        InterfaceInfo info = *it;
        if (info.name == name) {
            interfaces.erase(it);

            switch (info.interface) {
            case Registry::CompositorInterface:
                Q_EMIT q->compositorRemoved(name);
                break;
            case Registry::FullscreenShellInterface:
                Q_EMIT q->fullscreenShellRemoved(name);
                break;
            case Registry::OutputInterface:
                Q_EMIT q->outputRemoved(name);
                break;
            case Registry::ScreencasterInterface:
                Q_EMIT q->screencasterRemoved(name);
                break;
            case Registry::ScreenshooterInterface:
                Q_EMIT q->screenshooterRemoved(name);
                break;
            case Registry::SeatInterface:
                Q_EMIT q->seatRemoved(name);
                break;
            case Registry::ShmInterface:
                Q_EMIT q->shmRemoved(name);
                break;
            default:
                break;
            }

            break;
        }
    }

    Q_EMIT q->interfaceRemoved(name);

    if (interfaces.size() == 0)
        Q_EMIT q->interfacesRemoved();
}

void RegistryPrivate::handleSync()
{
    Q_Q(Registry);
    Q_EMIT q->interfacesAnnounced();
}

void RegistryPrivate::globalAnnounce(void *data, wl_registry *registry, uint32_t name,
                                     const char *interface, uint32_t version)
{
    auto self = reinterpret_cast<RegistryPrivate *>(data);
    Q_ASSERT(registry == self->registry);
    self->handleAnnounce(interface, name, version);
}

void RegistryPrivate::globalRemove(void *data, wl_registry *registry, uint32_t name)
{
    auto self = reinterpret_cast<RegistryPrivate *>(data);
    Q_ASSERT(registry == self->registry);
    self->handleRemove(name);
}

void RegistryPrivate::globalSync(void *data, wl_callback *callback, uint32_t serial)
{
    Q_UNUSED(serial)

    auto self = reinterpret_cast<RegistryPrivate *>(data);
    Q_ASSERT(callback == self->callback);
    self->handleSync();
    wl_callback_destroy(self->callback);
    self->callback = Q_NULLPTR;
}

const struct wl_registry_listener RegistryPrivate::s_registryListener = {
    globalAnnounce,
    globalRemove
};

const struct wl_callback_listener RegistryPrivate::s_callbackListener = {
    globalSync
};


/*
 * Registry
 */

Registry::Registry(QObject *parent)
    : QObject(*new RegistryPrivate(), parent)
{
}

bool Registry::isValid() const
{
    Q_D(const Registry);
    return d->registry != Q_NULLPTR;
}

wl_display *Registry::display() const
{
    Q_D(const Registry);
    return d->display;
}

void Registry::create(wl_display *display)
{
    Q_D(Registry);

    Q_ASSERT(display);
    Q_ASSERT(!isValid());
    d->display = display;
    d->registry = wl_display_get_registry(display);
    d->callback = wl_display_sync(display);
}

void Registry::setup()
{
    Q_D(Registry);
    Q_ASSERT(isValid());
    d->setup();
}

Compositor *Registry::createCompositor(quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    Compositor *compositor = new Compositor(parent);
    CompositorPrivate::get(compositor)->init(d->registry, name, version);
    return compositor;
}

FullScreenShell *Registry::createFullScreenShell(quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    FullScreenShell *fsh = new FullScreenShell(parent);
    FullScreenShellPrivate::get(fsh)->init(d->registry, name, version);
    return fsh;
}

Output *Registry::createOutput(quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    Output *output = new Output(parent);
    OutputPrivate::get(output)->init(d->registry, name, version);
    return output;
}

Seat *Registry::createSeat(quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    Seat *seat = new Seat(parent);
    SeatPrivate::get(seat)->init(d->registry, name, version);
    SeatPrivate::get(seat)->version = version;
    return seat;
}

Shm *Registry::createShm(quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    Shm *shm = new Shm(parent);
    ShmPrivate::get(shm)->init(d->registry, name, version);
    return shm;
}

Screencaster *Registry::createScreencaster(Shm *shm, quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    Screencaster *screencaster = new Screencaster(shm, parent);
    ScreencasterPrivate::get(screencaster)->registry = this;
    ScreencasterPrivate::get(screencaster)->init(d->registry, name, version);
    return screencaster;
}

Screenshooter *Registry::createScreenshooter(Shm *shm, quint32 name, quint32 version, QObject *parent)
{
    Q_D(Registry);
    Screenshooter *screenshooter = new Screenshooter(shm, parent);
    ScreenshooterPrivate::get(screenshooter)->registry = this;
    ScreenshooterPrivate::get(screenshooter)->init(d->registry, name, version);
    return screenshooter;
}

QByteArray Registry::interfaceName()
{
    return QByteArrayLiteral("wl_registry");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_registry.cpp"
