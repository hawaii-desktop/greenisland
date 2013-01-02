/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2010 Canonical, Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Olivier Tilloy <olivier.tilloy@canonical.com>
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

#include "launcheritem.h"

LauncherItem::LauncherItem()
    : QObject()
{
}

void LauncherItem::setActive(bool)
{
}

void LauncherItem::setRunning(bool)
{
}

bool LauncherItem::isCounterVisible() const
{
    return false;
}

int LauncherItem::counter() const
{
    return -1;
}

#include "moc_launcheritem.cpp"
