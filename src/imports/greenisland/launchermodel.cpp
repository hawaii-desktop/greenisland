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

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QStandardPaths>
#include <QStringBuilder>

#include <VBookmarkManager>

#include "launchermodel.h"
#include "launcherapplication.h"
#include "launcherurl.h"
#include "cmakedirs.h"

LauncherModel::LauncherModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Create a bookmark manager
    QString fileName = QString("%1/greenisland/launcher.xbel").arg(
                           QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    m_manager = VBookmarkManager::managerForFile(fileName, "Launcher");

    // If the bookmarks file doesn't exist, create initial items and save it
    VBookmarkGroup root = m_manager->root();
    if (root.first().isNull() || !QFile::exists(fileName)) {
        QStringList files;
        files << QStringLiteral("swordfish.desktop")
              << QStringLiteral("terminal.desktop")
              << QStringLiteral("system-preferences.desktop");

        foreach(QString file, files) {
            QString fileFound = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, file);
            QFileInfo fileInfo(fileFound);
            if (fileInfo.exists()) {
                VBookmark bookmark = createBookmark(fileInfo.baseName(),
                                                    fileInfo.filePath(),
                                                    false, false);
                m_list.append(new LauncherApplication(bookmark));
            }
        }
    } else {
        VBookmark bookmark = root.first();
        while (!bookmark.isNull()) {
            m_list.append(new LauncherApplication(bookmark));
            bookmark = root.next(bookmark);
        }
    }

    m_manager->saveAs(fileName, false);
}

QVariant LauncherModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!index.isValid())
        return QVariant();

    LauncherItem *item = m_list.at(index.row());
    if (item->type() == LauncherItem::ApplicationLauncherItem)
        return QVariant::fromValue(qobject_cast<LauncherApplication *>(item));
    else
        return QVariant::fromValue(qobject_cast<LauncherUrl *>(item));
}

int LauncherModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_list.size();
}

void LauncherModel::pinApplication(const QString &path)
{
    QFileInfo fileInfo(path);
    VBookmark bookmark = createBookmark(fileInfo.baseName(),
                                        fileInfo.filePath(),
                                        false, false);
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(new LauncherApplication(bookmark));
    endInsertRows();
}

void LauncherModel::pinUrl(const QUrl &url)
{
    VBookmark bookmark = createBookmark(url.toLocalFile(),
                                        url.toLocalFile(),
                                        false, false);
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(new LauncherUrl(bookmark));
    endInsertRows();
}

VBookmark LauncherModel::createBookmark(const QString &name,
                                        const QString &desktopFile,
                                        bool isEditable, bool isRemovable)
{
    Q_ASSERT(m_manager);

    VBookmarkGroup root = m_manager->root();
    if (root.isNull())
        return VBookmark();

    VBookmark bookmark = root.addBookmark(name, QUrl::fromLocalFile(desktopFile));
    bookmark.setMetaDataItem("IsEditable", isEditable ? "true" : "false");
    bookmark.setMetaDataItem("IsRemovable", isRemovable ? "true" : "false");
    return bookmark;
}

void LauncherModel::addItem(LauncherApplication *item)
{
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(item);
    endInsertRows();
}

LauncherApplication *LauncherModel::findItem(const QString &id, const QString &ver)
{
    foreach(LauncherItem * item, m_list) {
        LauncherApplication *app = qobject_cast<LauncherApplication *>(item);
        if (app && app->identifier() == id && app->version() == ver)
            return app;
    }

    return 0;
}

#include "moc_launchermodel.cpp"
