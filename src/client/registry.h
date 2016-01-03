/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLANDCLIENT_REGISTRY_H
#define GREENISLANDCLIENT_REGISTRY_H

#include <QtCore/QObject>

#include <GreenIsland/client/greenislandclient_export.h>

struct wl_display;

namespace GreenIsland {

namespace Client {

class Compositor;
class FullScreenShell;
class Output;
class RegistryPrivate;
class Screencaster;
class Screenshooter;
class Seat;
class Shm;

class GREENISLANDCLIENT_EXPORT Registry : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Registry)
public:
    enum Interface {
        UnknownInterface = 0,
        CompositorInterface,
        FullscreenShellInterface,
        OutputInterface,
        ScreencasterInterface,
        ScreenshooterInterface,
        SeatInterface,
        ShmInterface
    };
    Q_ENUM(Interface)

    Registry(QObject *parent = Q_NULLPTR);

    bool isValid() const;

    wl_display *display() const;

    void create(wl_display *display);
    void setup();

    Compositor *createCompositor(quint32 name, quint32 version, QObject *parent = Q_NULLPTR);
    FullScreenShell *createFullScreenShell(quint32 name, quint32 version, QObject *parent = Q_NULLPTR);
    Output *createOutput(quint32 name, quint32 version, QObject *parent = Q_NULLPTR);
    Seat *createSeat(quint32 name, quint32 version, QObject *parent = Q_NULLPTR);
    Shm *createShm(quint32 name, quint32 version, QObject *parent = Q_NULLPTR);

    Screencaster *createScreencaster(Shm *shm, quint32 name, quint32 version,
                                     QObject *parent = Q_NULLPTR);
    Screenshooter *createScreenshooter(Shm *shm, quint32 name, quint32 version,
                                       QObject *parent = Q_NULLPTR);

    static QByteArray interfaceName();

Q_SIGNALS:
    void interfaceAnnounced(const QByteArray &interface, quint32 name, quint32 version);
    void interfaceRemoved(quint32 name);

    void interfacesAnnounced();
    void interfacesRemoved();

    void compositorAnnounced(quint32 name, quint32 version);
    void compositorRemoved(quint32 name);

    void fullscreenShellAnnounced(quint32 name, quint32 version);
    void fullscreenShellRemoved(quint32 name);

    void outputAnnounced(quint32 name, quint32 version);
    void outputRemoved(quint32 name);

    void seatAnnounced(quint32 name, quint32 version);
    void seatRemoved(quint32 name);

    void shmAnnounced(quint32 name, quint32 version);
    void shmRemoved(quint32 name);

    void screencasterAnnounced(quint32 name, quint32 version);
    void screencasterRemoved(quint32 name);

    void screenshooterAnnounced(quint32 name, quint32 version);
    void screenshooterRemoved(quint32 name);
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_REGISTRY_H
