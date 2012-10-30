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

#ifndef LAUNCHERAPPLICATION_H
#define LAUNCHERAPPLICATION_H

#include "launcheritem.h"

class VApplicationInfo;
class VBookmark;

class LauncherApplication : public LauncherItem
{
    Q_OBJECT
    Q_PROPERTY(bool sticky READ sticky WRITE setSticky NOTIFY stickyChanged)
    Q_PROPERTY(QString desktopFile READ desktopFile WRITE setDesktopFile NOTIFY desktopFileChanged)
    Q_PROPERTY(QString identifier READ identifier)
    Q_PROPERTY(QString version READ version)
public:
    explicit LauncherApplication();
    LauncherApplication(const VBookmark &bookmark);
    ~LauncherApplication();

    virtual LauncherItem::ItemType type() const {
        return LauncherItem::ApplicationLauncherItem;
    }

    virtual QString label() const;
    virtual QString iconName() const;
    virtual QUrl url() const;

    QString identifier() const;
    QString version() const;

    virtual bool isActive() const;
    virtual void setActive(bool on);

    virtual bool isRunning() const;
    virtual void setRunning(bool on);

    virtual bool isUrgent() const;

    virtual bool isDraggable() const;

    virtual bool isEditable() const;

    virtual bool isRemovable() const;

    bool sticky() const;
    void setSticky(bool on);

    QString desktopFile() const;
    void setDesktopFile(const QString &desktopFile);

    Q_INVOKABLE virtual void activate();
    Q_INVOKABLE virtual void launchNewInstance();
    Q_INVOKABLE virtual void createMenuActions();

signals:
    void stickyChanged(bool);
    void desktopFileChanged(QString);

private:
    bool m_sticky;
    QString m_desktopFile;
    VApplicationInfo *m_appInfo;
    bool m_active;
    bool m_running;
    bool m_urgent;
    bool m_editable;
    bool m_removable;

    void initialize();

private slots:
    void slotNameChanged();
    void slotIconNameChanged();
    void slotActionTriggered();
};

Q_DECLARE_METATYPE(LauncherApplication *)

#endif // LAUNCHERAPPLICATION_H
