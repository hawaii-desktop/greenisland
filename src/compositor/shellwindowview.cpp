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

#include "output.h"
#include "shellwindowview.h"

namespace GreenIsland {

ShellWindowView::ShellWindowView(QWaylandQuickSurface *surface, Output *output,
                                 QQuickItem *parent)
    : QWaylandSurfaceItem(surface, parent)
    , m_role(NoneRole)
    , m_output(output)
{
}

ShellWindowView::Role ShellWindowView::role() const
{
    return m_role;
}

void ShellWindowView::setRole(const Role &role)
{
    if (m_role == role)
        return;

    m_role = role;
    Q_EMIT roleChanged();
}

ShellWindowView::Flags ShellWindowView::flags() const
{
    return m_flags;
}

void ShellWindowView::setFlags(const Flags &flags)
{
    if (m_flags == flags)
        return;

    m_flags = flags;
    Q_EMIT flagsChanged();
}

Output *ShellWindowView::output() const
{
    return m_output;
}

void ShellWindowView::setOutput(Output *output)
{
    if (m_output == output)
        return;

    m_output = output;
    Q_EMIT outputChanged();
}

}

#include "moc_shellwindowview.cpp"
