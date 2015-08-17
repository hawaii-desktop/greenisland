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

#ifndef GREENISLAND_CLIENT_WLREGISTRY_H
#define GREENISLAND_CLIENT_WLREGISTRY_H

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(WLREGISTRY)

struct wl_compositor;
struct wl_display;
struct wl_registry;
struct wl_seat;

namespace GreenIsland {

namespace Client {

class WlRegistryPrivate;
class WlShmPool;

class WlRegistry : public QObject
{
    Q_OBJECT
public:
    enum Interface {
        Unknown,
        Compositor,
        Seat,
        Shm,
        FullscreenShell
    };

    WlRegistry(QObject *parent = 0);
    ~WlRegistry();

    bool isValid() const;

    wl_registry *registry() const;

    void create(wl_display *display);
    void setup();

    wl_compositor *bindCompositor();

    WlShmPool *createShmPool(QObject *parent);

Q_SIGNALS:
    void interfaceAnnounced(const QString &interface, quint32 name, quint32 version);
    void interfaceRemoved(quint32 name);

    void interfacesAnnounced();
    void interfacesRemoved();

    void compositorAnnounced(quint32 name, quint32 version);
    void compositorRemoved(quint32 name);

    void seatAnnounced(quint32 name, quint32 version);
    void seatRemoved(quint32 name);

    void shmAnnounced(quint32 name, quint32 version);
    void shmRemoved(quint32 name);

    void fullscreenShellAnnounced(quint32 name, quint32 version);
    void fullscreenShellRemoved(quint32 name);

private:
    Q_DECLARE_PRIVATE(WlRegistry)
    WlRegistryPrivate *const d_ptr;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLAND_CLIENT_WLREGISTRY_H
