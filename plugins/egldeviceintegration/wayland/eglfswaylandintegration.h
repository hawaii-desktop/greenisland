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

#ifndef GREENISLAND_EGLFSWAYLANDINTEGRATION_H
#define GREENISLAND_EGLFSWAYLANDINTEGRATION_H

#include <QtGui/qpa/qplatformwindow.h>

#include <GreenIsland/Client/ClientConnection>
#include <GreenIsland/Client/Compositor>
#include <GreenIsland/Client/FullScreenShell>
#include <GreenIsland/Client/Keyboard>
#include <GreenIsland/Client/Pointer>
#include <GreenIsland/Client/Registry>
#include <GreenIsland/Client/Seat>
#include <GreenIsland/Client/Touch>

#include <GreenIsland/Platform/EGLDeviceIntegration>

#include "eglfswaylandinput.h"

namespace GreenIsland {

namespace Platform {

class EglFSWaylandScreen;

class EglFSWaylandIntegration : public QObject, public EGLDeviceIntegration
{
    Q_OBJECT
public:
    EglFSWaylandIntegration();

    void platformInit() Q_DECL_OVERRIDE;
    void platformDestroy() Q_DECL_OVERRIDE;

    EGLNativeDisplayType platformDisplay() const Q_DECL_OVERRIDE;

    bool handlesInput() Q_DECL_OVERRIDE;
    bool usesVtHandler() Q_DECL_OVERRIDE;
    bool usesDefaultScreen() Q_DECL_OVERRIDE;

    void screenInit() Q_DECL_OVERRIDE;

    QPlatformWindow *createPlatformWindow(QWindow *window) Q_DECL_OVERRIDE;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) Q_DECL_OVERRIDE;

    bool hasCapability(QPlatformIntegration::Capability cap) const Q_DECL_OVERRIDE;

    Client::Compositor *compositor() const { return m_compositor; }
    Client::FullScreenShell *fullScreenShell() const { return m_fullScreenShell; }

private:
    QThread *m_thread;
    Client::ClientConnection *m_connection;
    Client::Registry *m_registry;
    Client::Compositor *m_compositor;
    Client::FullScreenShell *m_fullScreenShell;
    Client::Seat *m_seat;
    QVector<Client::Output *> m_outputs;
    QVector<EglFSWaylandScreen *> m_screens;
    EglFSWaylandInput *m_input;
    QTouchDevice *m_touchDevice;

private Q_SLOTS:
    void keyboardAdded();
    void pointerAdded();
    void touchAdded();
    void touchRemoved();
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSWAYLANDINTEGRATION_H
