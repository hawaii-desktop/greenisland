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

#include <QDir>
#include <QDirIterator>
#include <QIcon>
#include <QStandardPaths>

#include <VFileSystemWatcher>

#include "appsmodel.h"

/*
 * AppsModel
 */

AppsModel::AppsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Watch for changes in application directories
    m_watcher = new VFileSystemWatcher(this);
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    foreach(QString dir, locations)
    m_watcher->addDir(dir);
    connect(m_watcher, SIGNAL(created(QString)),
            this, SLOT(slotFileCreated(QString)));
    connect(m_watcher, SIGNAL(deleted(QString)),
            this, SLOT(slotFileDeleted(QString)));
    connect(m_watcher, SIGNAL(dirty(QString)),
            this, SLOT(slotFileChanged(QString)));

    // Populate the list
    populate();
}

QHash<int, QByteArray> AppsModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NameRole] = "name";
    names[CommentRole] = "comment";
    names[IconNameRole] = "iconName";
    names[CategoriesRole] = "categories";
    return names;
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    VApplicationInfo *info = m_apps.at(index.row());

    switch (role) {
        case NameRole:
            return info->name();
        case CommentRole:
            return info->comment();
        case IconNameRole:
            return info->iconName();
        case CategoriesRole:
            return info->categories();
        default:
            break;
    }

    return QVariant();
}

int AppsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_apps.size();
}

void AppsModel::populate()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    foreach(QString path, paths) {
        QDirIterator walker(path,
                            QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                            QDirIterator::Subdirectories);
        while (walker.hasNext()) {
            walker.next();

            if (walker.fileInfo().completeSuffix() == "desktop") {
                // Add this item (only if it can be displayed)
                VApplicationInfo *info = new VApplicationInfo(walker.fileInfo().absoluteFilePath());
                if (info->isValid() && !info->isHidden() && info->isExecutable()) {
                    // Append item to the model
                    beginInsertRows(QModelIndex(), m_apps.size(), m_apps.size());
                    m_apps.append(info);
                    endInsertRows();

                    // Save categories
                    m_categories += QSet<QString>::fromList(info->categories());
                }
            }
        }
    }

    cleanupCategories();
}

void AppsModel::cleanupCategories()
{
    // Remove other desktop's categories
    m_categories.remove("Qt");
    m_categories.remove("KDE");
    m_categories.remove("GNOME");
}

void AppsModel::slotFileCreated(const QString &path)
{
    if (path.endsWith(".desktop") && QFile::exists(path)) {
        // A new file was created, add the item to the model
        VApplicationInfo *info = new VApplicationInfo(path);
        if (info->isValid() && !info->isHidden() && info->isExecutable()) {
            // Insert the new row
            beginInsertRows(QModelIndex(), m_apps.size(), m_apps.size());
            m_apps.append(info);
            endInsertRows();

            // Save catrgories
            m_categories += QSet<QString>::fromList(info->categories());
        }

        cleanupCategories();
    }
}

void AppsModel::slotFileDeleted(const QString &path)
{
    if (path.endsWith(".desktop") && QFile::exists(path)) {
        foreach(VApplicationInfo * info, m_apps) {
            // Remove row of the deleted file
            int row = m_apps.indexOf(info);
            if (info->fileName() == path) {
                beginRemoveRows(QModelIndex(), row, row);
                m_apps.removeAt(row);
                endRemoveRows();
                break;
            }
        }
    }

    // TODO: if app categories were only defined by the deleted .desktop file
    // we have to remove them from m_categories
}

void AppsModel::slotFileChanged(const QString &path)
{
    if (path.endsWith(".desktop") && QFile::exists(path)) {
        foreach(VApplicationInfo * info, m_apps) {
            if (info->fileName() == path) {
                // We found the changed application info
                int row = m_apps.indexOf(info);
                VApplicationInfo *newInfo = new VApplicationInfo(path);

                // If application is now not valid, hidden or not executable we remove
                // it from the model, otherwise we update it
                if (!newInfo->isValid() || newInfo->isHidden() || !newInfo->isExecutable()) {
                    beginRemoveRows(QModelIndex(), row, row);
                    m_apps.removeAt(row);
                    endRemoveRows();
                } else {
                    m_apps.replace(row, newInfo);
                    emit dataChanged(index(row), index(row));
                }
            }
        }
    }

    // TODO: reload its categories
}

/*
 * AppsModelSorted
 */

AppsModelSorted::AppsModelSorted(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    m_model = new AppsModel();
    setSourceModel(m_model);
    setSortRole(AppsModel::NameRole);
    setSortLocaleAware(true);
    setSortCaseSensitivity(Qt::CaseSensitive);
    setDynamicSortFilter(true);
}

AppsModelSorted::~AppsModelSorted()
{
    delete m_model;
}

#include "moc_appsmodel.cpp"
