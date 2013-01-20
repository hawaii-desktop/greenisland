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

#include <QList>
#include <QPair>

#include "appchoosercategories.h"

AppChooserCategories::AppChooserCategories(QObject *parent)
    : QAbstractListModel(parent)
{
    m_categories.append(AppChooserCategoryPair("AudioVideo", tr("Sound & Video")));
    m_categories.append(AppChooserCategoryPair("Audio", tr("Audio")));
    m_categories.append(AppChooserCategoryPair("Video", tr("Video")));
    m_categories.append(AppChooserCategoryPair("Development", tr("Programming")));
    m_categories.append(AppChooserCategoryPair("Education", tr("Education")));
    m_categories.append(AppChooserCategoryPair("Game", tr("Game")));
    m_categories.append(AppChooserCategoryPair("Graphics", tr("Graphics")));
    m_categories.append(AppChooserCategoryPair("Network", tr("Network")));
    m_categories.append(AppChooserCategoryPair("Office", tr("Office")));
    m_categories.append(AppChooserCategoryPair("Science", tr("Science")));
    m_categories.append(AppChooserCategoryPair("Settings", tr("Settings")));
    m_categories.append(AppChooserCategoryPair("System", tr("System")));
    m_categories.append(AppChooserCategoryPair("Utility", tr("Utilities")));
}


QHash<int, QByteArray> AppChooserCategories::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[LabelRole] = "label";
    return roles;
}

QVariant AppChooserCategories::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
        case LabelRole:
        case Qt::DisplayRole:
            return m_categories.at(index.row()).second;
        case NameRole:
            return m_categories.at(index.row()).first;
        default:
            break;
    }

    return QVariant();
}

int AppChooserCategories::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_categories.size();
}

#include "moc_appchoosercategories.cpp"
