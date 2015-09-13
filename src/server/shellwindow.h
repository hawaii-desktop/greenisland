/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef SHELLWINDOW_H
#define SHELLWINDOW_H

#include <QtCore/QObject>

#include <greenisland/greenisland_export.h>

class QWaylandOutput;
class QWaylandSurface;
class QWaylandSurfaceItem;

namespace GreenIsland {

class Compositor;

class GREENISLAND_EXPORT ShellWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint id READ id CONSTANT)
    Q_PROPERTY(QWaylandSurface *surface READ surface CONSTANT)
    Q_PROPERTY(QWaylandSurfaceItem *view READ view CONSTANT)
    Q_PROPERTY(Role role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(Flags flags READ flags WRITE setFlags NOTIFY flagsChanged)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
    Q_ENUMS(Role Flag)
public:
    enum Role {
        UnknownRole = 0,
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

    ShellWindow(QWaylandSurface *surface, QObject *parent = 0);
    ~ShellWindow();

    uint id() const;

    QWaylandSurface *surface() const;
    QWaylandSurfaceItem *view() const;

    QWaylandOutput *output() const;

    Role role() const;
    void setRole(const Role &role);

    Flags flags() const;
    void setFlags(const Flags &flags);

Q_SIGNALS:
    void roleChanged();
    void flagsChanged();
    void outputChanged();
    void moveRequested(const QPointF &from, const QPointF &to);
    void moveRequested(const QPointF &to);

private:
    Role m_role;
    Flags m_flags;
    Compositor *m_compositor;
    QWaylandSurface *m_surface;
    QWaylandSurfaceItem *m_view;

    void registerWindow();
    void unregisterWindow(bool destruction);

    static uint m_id;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShellWindow::Flags)

}

#endif // SHELLWINDOW_H
