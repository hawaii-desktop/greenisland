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
