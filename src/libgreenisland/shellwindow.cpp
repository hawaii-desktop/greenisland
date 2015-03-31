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

#include "compositor.h"
#include "compositor_p.h"
#include "shellwindow.h"

namespace GreenIsland {

uint ShellWindow::m_id = 0;

ShellWindow::ShellWindow(QWaylandSurface *surface, QObject *parent)
    : QObject(parent)
    , m_role(UnknownRole)
    , m_flags(0)
    , m_compositor(static_cast<Compositor *>(surface->compositor()))
    , m_surface(surface)
{
    qRegisterMetaType<ShellWindow *>("ShellWindow*");

    // Identifier
    m_id++;

    // Create view
    m_view = new QWaylandSurfaceItem(static_cast<QWaylandQuickSurface *>(surface));

    // Connect to surface signals
    connect(surface, &QWaylandSurface::mapped, [=] {
        registerWindow();
    });
    connect(surface, &QWaylandSurface::unmapped, [=] {
        unregisterWindow(false);
    });
}

ShellWindow::~ShellWindow()
{
    unregisterWindow(true);
    m_view->deleteLater();
}

uint ShellWindow::id() const
{
    return m_id;
}

QWaylandSurface *ShellWindow::surface() const
{
    return m_surface;
}

QWaylandSurfaceItem *ShellWindow::view() const
{
    return m_view;
}

QWaylandOutput *ShellWindow::output() const
{
    return m_surface->mainOutput();
}

ShellWindow::Role ShellWindow::role() const
{
    return m_role;
}

void ShellWindow::setRole(const Role &role)
{
    if (m_role == role)
        return;

    m_role = role;
    Q_EMIT roleChanged();
}

ShellWindow::Flags ShellWindow::flags() const
{
    return m_flags;
}

void ShellWindow::setFlags(const Flags &flags)
{
    if (m_flags == flags)
        return;

    m_flags = flags;
    Q_EMIT flagsChanged();
}

void ShellWindow::registerWindow()
{
    // Register this window
    m_compositor->d_func()->mapShellWindow(this);
}

void ShellWindow::unregisterWindow(bool destruction)
{
    // Unregister this window
    if (destruction)
        m_compositor->d_func()->destroyShellWindow(this);
    else
        m_compositor->d_func()->unmapShellWindow(this);
}

}

#include "moc_shellwindow.cpp"
