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

#ifndef LAUNCHERURL_H
#define LAUNCHERURL_H

#include <VBookmark>

#include "launcheritem.h"

class LauncherUrl : public LauncherItem
{
    Q_OBJECT
public:
    explicit LauncherUrl();
    LauncherUrl(const VBookmark &bookmark);

    virtual LauncherItem::ItemType type() const {
        return LauncherItem::UrlLauncherItem;
    }

    virtual QString label() const;
    virtual QString iconName() const;
    virtual QUrl url() const;

    virtual bool isActive() const;
    virtual bool isRunning() const;
    virtual bool isUrgent() const;
    virtual bool isDraggable() const;
    virtual bool isEditable() const;
    virtual bool isRemovable() const;

    Q_INVOKABLE virtual void activate();
    Q_INVOKABLE virtual void launchNewInstance();
    Q_INVOKABLE virtual void createMenuActions();

private:
    VBookmark m_bookmark;
    bool m_active;
    bool m_running;

    void initialize();

private slots:
    void slotActionTriggered();
};

Q_DECLARE_METATYPE(LauncherUrl *)

#endif // LAUNCHERURL_H
