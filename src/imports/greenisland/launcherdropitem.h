/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2011 Canonical, Ltd.
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

#ifndef LAUNCHERDROPITEM_H
#define LAUNCHERDROPITEM_H

#include <QQuickItem>

class LauncherDropItem : public QQuickItem
{
    Q_OBJECT
public:
    LauncherDropItem(QQuickItem *parent = 0);

signals:
    void applicationDropped(QString path);
    void urlDropped(const QUrl &url);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QList<QUrl> getEventUrls(QDragEnterEvent *event);
};

#endif // LAUNCHERDROPITEM_H
