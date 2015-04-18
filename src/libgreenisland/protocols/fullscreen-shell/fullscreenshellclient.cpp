/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtGui/QGuiApplication>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QScreen>

#include "fullscreenshellclient.h"
#include "output.h"

Q_LOGGING_CATEGORY(FSH_CLIENT_PROTOCOL, "greenisland.protocols.fullscreenshell.client")

namespace GreenIsland {

FullScreenShellClient::FullScreenShellClient(wl_registry *registry, quint32 name, quint32 version)
    : QtWayland::_wl_fullscreen_shell(registry, name, version)
    , m_capabilities(0)
{
}


FullScreenShellClient::Capabilities FullScreenShellClient::capabilities() const
{
    return m_capabilities;
}

void FullScreenShellClient::showOutput(Output *output)
{
    if (!output || !output->window()) {
        qCWarning(FSH_CLIENT_PROTOCOL) << "Cannot show a null output window!";
        return;
    }

    QPlatformNativeInterface *native =
            QGuiApplication::platformNativeInterface();
    if (!native)
        qFatal("Platform native interface not found, aborting...");

    wl_surface *wlSurface = static_cast<wl_surface*>(
                native->nativeResourceForWindow("surface", output->window()));
    if (!wlSurface)
        qFatal("Unable to get wl_surface from output window, aborting...");

    wl_output *wlOutput = static_cast<wl_output *>(
                native->nativeResourceForScreen("output", output->window()->screen()));
    if (!wlOutput)
        qFatal("Unable to get wl_output from output, aborting...");

    present_surface(wlSurface, present_method_default, wlOutput);
}

void FullScreenShellClient::hideOutput(Output *output)
{
    if (!output || !output->window()) {
        qCWarning(FSH_CLIENT_PROTOCOL) << "Cannot hide a null output window!";
        return;
    }

    QPlatformNativeInterface *native =
            QGuiApplication::platformNativeInterface();
    if (!native)
        qFatal("Platform native interface not found, aborting...");

    wl_output *wlOutput = static_cast<wl_output *>(
                native->nativeResourceForScreen("output", output->window()->screen()));
    if (!wlOutput)
        qFatal("Unable to get wl_output from output, aborting...");

    present_surface(Q_NULLPTR, present_method_default, wlOutput);
}

void FullScreenShellClient::fullscreen_shell_capability(uint32_t capability)
{
    switch (capability) {
    case QtWayland::_wl_fullscreen_shell::capability_arbitrary_modes:
        m_capabilities |= FullScreenShellClient::ArbitraryModes;
        break;
    case QtWayland::_wl_fullscreen_shell::capability_cursor_plane:
        m_capabilities |= FullScreenShellClient::CursorPlane;
        break;
    default:
        break;
    }
}

}
