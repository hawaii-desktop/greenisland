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

#ifndef FULLSCREENSHELLCLIENT_H
#define FULLSCREENSHELLCLIENT_H

#include "qwayland-fullscreen-shell.h"

struct wl_registry;

class Output;

class FullScreenShellClient : public QtWayland::_wl_fullscreen_shell
{
public:
    enum Capability {
        ArbitraryModes = 1,
        CursorPlane
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    explicit FullScreenShellClient(quint32 id, quint32 version);

    quint32 id() const;

    Capabilities capabilities() const;

    void showOutput(Output *output);
    void hideOutput(Output *output);

private:
    quint32 m_id;
    Capabilities m_capabilities;

    void fullscreen_shell_capability(uint32_t capabilty) Q_DECL_OVERRIDE;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FullScreenShellClient::Capabilities)

#endif // FULLSCREENSHELLCLIENT_H
