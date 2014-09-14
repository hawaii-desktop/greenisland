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

#ifndef SHELLWINDOWVIEW_H
#define SHELLWINDOWVIEW_H

#include <QtCompositor/QWaylandSurfaceItem>

namespace GreenIsland {

class Output;

class ShellWindowView : public QWaylandSurfaceItem
{
    Q_OBJECT
    Q_PROPERTY(Role role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(Output *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_ENUMS(Role)
public:
    enum Role {
        NoneRole = 0,
        DesktopRole,
        DashboardRole,
        PanelRole,
        PanelConfigRole,
        OverlayRole,
        NotificationRole,
        LockRole
    };

    explicit ShellWindowView(QWaylandQuickSurface *surface, Output *output,
                             QQuickItem *parent = 0);

    Role role() const;
    void setRole(const Role &role);

    Output *output() const;
    void setOutput(Output *output);

Q_SIGNALS:
    void roleChanged();
    void outputChanged();

private:
    Role m_role;
    Output *m_output;
};

}

#endif // SHELLWINDOWVIEW_H
