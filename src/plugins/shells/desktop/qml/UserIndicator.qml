/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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

import QtQuick 2.0
import GreenIsland 1.0

PanelIndicator {
    property int userStatus: UserStatus.Offline

    iconName: {
        switch (userStatus) {
        case UserStatus.Offline:
            return "user-offline-symbolic";
        case UserStatus.Available:
            return "user-available-symbolic";
        case UserStatus.Busy:
            return "user-busy-symbolic";
        case UserStatus.Invisible:
            return "user-invisible-symbolic";
        case UserStatus.Away:
            return "user-away-symbolic";
        case UserStatus.Idle:
            return "user-idle-symbolic";
        case UserStatus.Pending:
            return "user-status-pending-symbolic";
        case UserStatus.Locked:
            return "changes-prevent-symbolic";
        }
    }
    label: "Pier Luigi Fiorini"

    menu: PanelMenu {
        content: [
            PanelMenuItem {
                text: qsTr("Disconnect")
                onClicked: shell.disconnectUser()
            }
        ]
    }
}
