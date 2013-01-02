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

#include <QMimeData>
#include <QDebug>

#include "launcherdropitem.h"

LauncherDropItem::LauncherDropItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    // Make this a QML FocusScope
    setFlags(QQuickItem::ItemIsFocusScope | QQuickItem::ItemAcceptsDrops);
}

void LauncherDropItem::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << event;
    foreach(const QUrl & url, getEventUrls(event)) {
        qDebug() << url;
        if ((url.scheme() == "file" && url.path().endsWith(".desktop")) ||
                (url.scheme().startsWith("file") || url.scheme().startsWith("http") || url.scheme().startsWith("https"))) {
            event->setAccepted(true);
            return;
        }
    }

    event->setAccepted(false);
}

void LauncherDropItem::dropEvent(QDropEvent *event)
{
    qDebug() << event;
    foreach(const QUrl & url, getEventUrls(reinterpret_cast<QDragEnterEvent *>(event))) {
        qDebug() << url;
        if (url.scheme() == "file" && url.path().endsWith(".desktop"))
            emit applicationDropped(url.path());
        else if (url.scheme().startsWith("file") || url.scheme().startsWith("http") || url.scheme().startsWith("https"))
            emit urlDropped(url);
    }

    event->setAccepted(false);
}

QList<QUrl> LauncherDropItem::getEventUrls(QDragEnterEvent *event)
{
    qDebug() << event;
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
        return mimeData->urls();
    else if (mimeData->hasText()) {
        /*
         * When dragging an URL from some programs like Chromium it
         * is recognized as plain text, so we try to build
         * an URL out of this text.
         */
        QString text = mimeData->text();
        QUrl url(text);
        qDebug() << url;
        if (url.isRelative())
            // On top of that, Chromium sometimes chops off the scheme...
            url = QUrl("http://" + text);

        if (url.isValid()) {
            QList<QUrl> urls;
            urls.append(url);
            return urls;
        }
    }

    return QList<QUrl>();
}

#include "moc_launcherdropitem.cpp"
