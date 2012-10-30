/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 * Copyright (c) 2010 Canonical, Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef LAUNCHERITEM_H
#define LAUNCHERITEM_H

#include <QObject>
#include <QUrl>

class LauncherItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ItemType type READ type)
    Q_PROPERTY(QString label READ label NOTIFY labelChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setActive NOTIFY runningChanged)
    Q_PROPERTY(bool urgent READ isUrgent NOTIFY urgentChanged)
    Q_PROPERTY(bool draggable READ isDraggable NOTIFY draggableChanged)
    Q_PROPERTY(bool editable READ isEditable NOTIFY editableChanged)
    Q_PROPERTY(bool removable READ isRemovable NOTIFY removableChanged)
    Q_PROPERTY(bool counterVisible READ isCounterVisible NOTIFY counterVisibleChanged)
    Q_PROPERTY(int counter READ counter NOTIFY counterChanged)
    Q_ENUMS(ItemType)
public:
    enum ItemType {
        ApplicationLauncherItem,
        UrlLauncherItem,
        SpecialLauncherItem
    };

    LauncherItem();

    virtual ItemType type() const = 0;

    virtual QString label() const = 0;
    virtual QString iconName() const = 0;
    virtual QUrl url() const = 0;

    virtual bool isActive() const = 0;
    virtual void setActive(bool on);

    virtual bool isRunning() const = 0;
    virtual void setRunning(bool on);

    virtual bool isUrgent() const = 0;
    virtual bool isDraggable() const = 0;
    virtual bool isEditable() const = 0;
    virtual bool isRemovable() const = 0;

    virtual bool isCounterVisible() const;
    virtual int counter() const;

    Q_INVOKABLE virtual void activate() = 0;
    Q_INVOKABLE virtual void launchNewInstance() = 0;
    Q_INVOKABLE virtual void createMenuActions() = 0;

signals:
    void labelChanged(const QString &label);
    void iconNameChanged(const QString &iconName);
    void urlChanged(const QUrl &url);
    void activeChanged(bool active);
    void runningChanged(bool running);
    void urgentChanged(bool urgent);
    void draggableChanged(bool draggable);
    void editableChanged(bool editable);
    void removableChanged(bool removable);
    void counterVisibleChanged(bool value);
    void counterChanged(int value);
};

#endif // LAUNCHERITEM_H
