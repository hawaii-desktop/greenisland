/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
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

#include "launcherappchooser.h"

LauncherAppChooser::LauncherAppChooser()
    : LauncherItem()
    , m_isActive(false)
{
}

QString LauncherAppChooser::label() const
{
    return tr("AppChooser");
}

QString LauncherAppChooser::iconName() const
{
    return "view-grid-symbolic";
}

QUrl LauncherAppChooser::url() const
{
    return QUrl();
}

bool LauncherAppChooser::isActive() const
{
    return m_isActive;
}

bool LauncherAppChooser::isRunning() const
{
    return false;
}

bool LauncherAppChooser::isUrgent() const
{
    return false;
}

bool LauncherAppChooser::isDraggable() const
{
    return false;
}

bool LauncherAppChooser::isEditable() const
{
    return false;
}

bool LauncherAppChooser::isRemovable() const
{
    return false;
}

void LauncherAppChooser::activate()
{
}

void LauncherAppChooser::launchNewInstance()
{
}

void LauncherAppChooser::createMenuActions()
{
}

#include "moc_launcherappchooser.cpp"
