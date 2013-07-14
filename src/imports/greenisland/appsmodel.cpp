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

#include <QDir>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QStandardPaths>

#include "appsmodel.h"

/*
 * AppsModel
 */

AppsModel::AppsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Watch for changes in application directories
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPaths(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation));
    connect(m_watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChanged(QString)));

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

    QApplicationInfo *info = m_apps.at(index.row());

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

void AppsModel::launchApplicationAt(int index)
{
    QApplicationInfo *info = m_apps.at(index);
    if (info)
        info->launch(QStringList());
}

void AppsModel::populate()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (int i = 0; i < paths.size(); i++)
        directoryChanged(paths.at(i));

    cleanupCategories();
}

void AppsModel::cleanupCategories()
{
    // Remove other desktop's categories
    m_categories.remove("Qt");
    m_categories.remove("KDE");
    m_categories.remove("GNOME");
}

void AppsModel::directoryChanged(const QString &path)
{
    // Remove items from this path
    for (int i = 0; i < m_apps.size(); i++) {
        QApplicationInfo *info = m_apps.at(i);

        if (info->fileName().startsWith(path)) {
            beginRemoveRows(QModelIndex(), i, i);
            m_apps.removeAt(i);
            delete info;
            endRemoveRows();
        }
    }

    QDirIterator walker(path,
                        QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                        QDirIterator::Subdirectories);
    while (walker.hasNext()) {
        walker.next();

        if (walker.fileInfo().completeSuffix() == "desktop") {
            QString fullPath = walker.fileInfo().absoluteFilePath();

            // Add this item (only if it can be displayed)
            QApplicationInfo *info = new QApplicationInfo(fullPath);
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
