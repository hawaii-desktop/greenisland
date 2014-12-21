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

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class Output;

class GREENISLAND_EXPORT ShellWindowView : public QWaylandSurfaceItem
{
    Q_OBJECT
    Q_PROPERTY(Role role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(Flags flags READ flags WRITE setFlags NOTIFY flagsChanged)
    Q_PROPERTY(Output *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_ENUMS(Role Flag)
public:
    enum Role {
        NoneRole = 0,
        SplashRole,
        DesktopRole,
        DashboardRole,
        PanelRole,
        OverlayRole,
        NotificationRole,
        LockRole
    };

    enum Flag {
        PanelAlwaysVisible = 1,
        PanelAutoHide,
        PanelWindowsCanCover,
        PanelWindowsGoBelow
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit ShellWindowView(QWaylandQuickSurface *surface, Output *output,
                             QQuickItem *parent = 0);

    Role role() const;
    void setRole(const Role &role);

    Flags flags() const;
    void setFlags(const Flags &flags);

    Output *output() const;
    void setOutput(Output *output);

Q_SIGNALS:
    void roleChanged();
    void flagsChanged();
    void outputChanged();

private:
    Role m_role;
    Flags m_flags;
    Output *m_output;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShellWindowView::Flags)

}

#endif // SHELLWINDOWVIEW_H
