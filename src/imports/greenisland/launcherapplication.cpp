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

#include <QAction>
#include <QStringList>

#include <VApplicationInfo>
#include <VBookmark>

#include "launcherapplication.h"

LauncherApplication::LauncherApplication()
    : LauncherItem()
{
    initialize();
}

LauncherApplication::LauncherApplication(const VBookmark &bookmark)
    : LauncherItem()
{
    initialize();

    m_sticky = true;
    m_editable = bookmark.metaDataItem("IsEditable") == "true";
    m_removable = bookmark.metaDataItem("IsRemovable") == "true";

    setDesktopFile(bookmark.url().toLocalFile());
}

LauncherApplication::~LauncherApplication()
{
    delete m_appInfo;
}

QString LauncherApplication::label() const
{
    if (m_appInfo)
        return m_appInfo->name();
    return QString();
}

QString LauncherApplication::iconName() const
{
    if (m_appInfo)
        return m_appInfo->iconName();
    return QString();
}

QUrl LauncherApplication::url() const
{
    if (m_appInfo)
        return QUrl::fromLocalFile(m_desktopFile);
    return QUrl();
}

QString LauncherApplication::identifier() const
{
    if (m_appInfo)
        return m_appInfo->identifier();
    return QString();
}

QString LauncherApplication::version() const
{
    if (m_appInfo)
        return m_appInfo->version();
    return QString();
}

bool LauncherApplication::isActive() const
{
    return m_active;
}

void LauncherApplication::setActive(bool on)
{
    m_active = on;
    emit activeChanged(on);
}

bool LauncherApplication::isRunning() const
{
    return m_running;
}

void LauncherApplication::setRunning(bool on)
{
    m_running = on;
    emit runningChanged(on);
}

bool LauncherApplication::isUrgent() const
{
    return m_urgent;
}

bool LauncherApplication::isDraggable() const
{
    return true;
}

bool LauncherApplication::isEditable() const
{
    return m_editable;
}

bool LauncherApplication::isRemovable() const
{
    return m_removable;
}

bool LauncherApplication::sticky() const
{
    return m_sticky;
}

void LauncherApplication::setSticky(bool on)
{
    if (m_sticky != on) {
        m_sticky = on;
        emit stickyChanged(m_sticky);
    }
}

QString LauncherApplication::desktopFile() const
{
    return m_desktopFile;
}

void LauncherApplication::setDesktopFile(const QString &desktopFile)
{
    if (m_desktopFile != desktopFile) {
        m_desktopFile = desktopFile;
        emit desktopFileChanged(m_desktopFile);

        delete m_appInfo;
        m_appInfo = new VApplicationInfo(m_desktopFile);
        connect(m_appInfo, SIGNAL(nameChanged(QString)),
                this, SLOT(slotNameChanged()));
        connect(m_appInfo, SIGNAL(iconNameChanged(QString)),
                this, SLOT(slotIconNameChanged()));
    }
}

void LauncherApplication::activate()
{
    if (!m_running) {
        // Run application if it's not running yet
        bool executed = m_appInfo->launch(QStringList());
        if (executed) {
            if (!m_running) {
                m_running = true;
                emit runningChanged(m_running);
            }
            if (!m_active) {
                m_active = true;
                emit activeChanged(m_active);
            }
        }
    } else {
        // If it's already running make it active
        // TODO:
        m_active = true;
    }
}

void LauncherApplication::launchNewInstance()
{
    if (m_appInfo)
        m_appInfo->launch(QStringList());
}

void LauncherApplication::createMenuActions()
{
#if 0
    QAction *action = m_menu->addAction(tr("Open"));
    connect(action, SIGNAL(triggered()),
            this, SLOT(slotActionTriggered()));
#endif
}

void LauncherApplication::initialize()
{
    m_sticky = false;
    m_appInfo = 0;
    m_active = false;
    m_running = false;
    m_urgent = false;
    m_editable = false;
    m_removable = false;
}

void LauncherApplication::slotNameChanged()
{
    emit labelChanged(m_appInfo->name());
}

void LauncherApplication::slotIconNameChanged()
{
    emit iconNameChanged(m_appInfo->iconName());
}

void LauncherApplication::slotActionTriggered()
{
}

#include "moc_launcherapplication.cpp"
