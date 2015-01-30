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

#include <QtCompositor/QWaylandOutput>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandSurfaceItem>

#include "shellwindow.h"

namespace GreenIsland {

ShellWindow::ShellWindow(QWaylandSurface *surface, QObject *parent)
    : QObject(parent)
    , m_role(UnknownRole)
    , m_flags(0)
    , m_surface(surface)
{
    qRegisterMetaType<ShellWindow *>("ShellWindow*");

    // Create view
    m_view = new QWaylandSurfaceItem(static_cast<QWaylandQuickSurface *>(surface));
}

ShellWindow::~ShellWindow()
{
    m_view->deleteLater();
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
    return m_surface->output();
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

}

#include "moc_shellwindow.cpp"
