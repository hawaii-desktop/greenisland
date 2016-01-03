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

#ifndef GREENISLAND_EGLFSX11INTEGRATION_H
#define GREENISLAND_EGLFSX11INTEGRATION_H

#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatformwindow.h>

#include <GreenIsland/Platform/EGLDeviceIntegration>

#include <xcb/xcb.h>

namespace GreenIsland {

namespace Platform {

namespace Atoms {
    enum {
        _NET_WM_NAME = 0,
        UTF8_STRING,
        WM_PROTOCOLS,
        WM_DELETE_WINDOW,
        _NET_WM_STATE,
        _NET_WM_STATE_FULLSCREEN,

        N_ATOMS
    };
}

class EventReader;

class EglFSX11Integration : public EGLDeviceIntegration
{
public:
    EglFSX11Integration() : m_connection(0), m_window(0), m_eventReader(0) {}

    void platformInit() Q_DECL_OVERRIDE;
    void platformDestroy() Q_DECL_OVERRIDE;
    EGLNativeDisplayType platformDisplay() const Q_DECL_OVERRIDE;

    bool usesVtHandler() Q_DECL_OVERRIDE;
    bool handlesInput() Q_DECL_OVERRIDE;

    QSize screenSize() const Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeWindow(QPlatformWindow *window,
                                           const QSize &size,
                                           const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    void destroyNativeWindow(EGLNativeWindowType window) Q_DECL_OVERRIDE;
    bool hasCapability(QPlatformIntegration::Capability cap) const Q_DECL_OVERRIDE;

    xcb_connection_t *connection() { return m_connection; }
    const xcb_atom_t *atoms() const { return m_atoms; }
    QPlatformWindow *platformWindow() { return m_platformWindow; }

private:
    void sendConnectionEvent(xcb_atom_t a);

    void *m_display;
    xcb_connection_t *m_connection;
    xcb_atom_t m_atoms[Atoms::N_ATOMS];
    xcb_window_t m_window;
    EventReader *m_eventReader;
    xcb_window_t m_connectionEventListener;
    QPlatformWindow *m_platformWindow;
    mutable QSize m_screenSize;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSX11INTEGRATION_H
