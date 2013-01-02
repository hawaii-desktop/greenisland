/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#include <QAction>
#include <QDesktopServices>
#include <QStringList>

#include "launcherurl.h"

LauncherUrl::LauncherUrl()
    : LauncherItem()
{
    initialize();
}

LauncherUrl::LauncherUrl(const VBookmark &bookmark)
    : LauncherItem()
{
    initialize();
    m_bookmark = bookmark;
}

QString LauncherUrl::label() const
{
    return m_bookmark.text();
}

QString LauncherUrl::iconName() const
{
    return m_bookmark.icon();
}

QUrl LauncherUrl::url() const
{
    return m_bookmark.url();
}

bool LauncherUrl::isActive() const
{
    return m_active;
}

bool LauncherUrl::isRunning() const
{
    return m_running;
}

bool LauncherUrl::isUrgent() const
{
    return false;
}

bool LauncherUrl::isDraggable() const
{
    return false;
}

bool LauncherUrl::isEditable() const
{
    return m_bookmark.metaDataItem("IsEditable") == "true";
}

bool LauncherUrl::isRemovable() const
{
    return m_bookmark.metaDataItem("IsRemovable") == "true";
}

void LauncherUrl::activate()
{
    // TODO:
    m_active = true;
    emit activeChanged(m_active);
}

void LauncherUrl::launchNewInstance()
{
    QDesktopServices::openUrl(url());
}

void LauncherUrl::createMenuActions()
{
#if 0
    QAction *action = m_menu->addAction(tr("Open"));
    connect(action, SIGNAL(triggered()),
            this, SLOT(slotActionTriggered()));
#endif
}

void LauncherUrl::initialize()
{
    m_active = false;
    m_running = false;
}

void LauncherUrl::slotActionTriggered()
{
}

#include "moc_launcherurl.cpp"
