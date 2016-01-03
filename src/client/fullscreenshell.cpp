/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2016 Pier Luigi Fiorini
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

#include "fullscreenshell.h"
#include "fullscreenshell_p.h"
#include "output.h"
#include "output_p.h"
#include "surface.h"
#include "surface_p.h"

Q_LOGGING_CATEGORY(FSH_CLIENT_PROTOCOL, "greenisland.protocols.fullscreenshell.client")

namespace GreenIsland {

namespace Client {

/*
 * FullScreenShellPrivate
 */

FullScreenShellPrivate::FullScreenShellPrivate()
    : QtWayland::_wl_fullscreen_shell()
    , capabilities(FullScreenShell::NoCapability)
{
}

void FullScreenShellPrivate::fullscreen_shell_capability(uint32_t capability)
{
    Q_Q(FullScreenShell);

    FullScreenShell::Capabilities oldCapabilities = capabilities;

    if (capability & QtWayland::_wl_fullscreen_shell::capability_arbitrary_modes)
        capabilities |= FullScreenShell::ArbitraryModes;
    if (capability & QtWayland::_wl_fullscreen_shell::capability_cursor_plane)
        capabilities |= FullScreenShell::CursorPlane;

    if (oldCapabilities != capabilities)
        Q_EMIT q->capabilitiesChanged();
}

/*
 * FullScreenShell
 */

FullScreenShell::FullScreenShell(QObject *parent)
    : QObject(parent)
{
}


FullScreenShell::Capabilities FullScreenShell::capabilities() const
{
    Q_D(const FullScreenShell);
    return d->capabilities;
}

void FullScreenShell::presentSurface(Surface *surface, Output *output, PresentMethod method)
{
    Q_D(FullScreenShell);
    d->present_surface(SurfacePrivate::get(surface)->object(),
                       static_cast<uint32_t>(method),
                       OutputPrivate::get(output)->object());
}

void FullScreenShell::hideOutput(Output *output)
{
    Q_D(FullScreenShell);
    d->present_surface(Q_NULLPTR,
                       QtWayland::_wl_fullscreen_shell::present_method_default,
                       OutputPrivate::get(output)->object());
}

} // namespace Client

} // namespace GreenIsland

#include "moc_fullscreenshell.cpp"
