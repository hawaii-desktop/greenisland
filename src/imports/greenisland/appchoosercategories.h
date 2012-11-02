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

#ifndef APPCHOOSERCATEGORIES_H
#define APPCHOOSERCATEGORIES_H

#include <QAbstractListModel>

class AppChooserCategories : public QAbstractListModel
{
    Q_OBJECT
public:
    enum {
        NameRole = Qt::UserRole + 1,
        LabelRole
    };

    explicit AppChooserCategories(QObject *parent = 0);

    QHash<int, QByteArray> roleNames() const;

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;

private:
    typedef QPair<QString, QString> AppChooserCategoryPair;
    QList<AppChooserCategoryPair> m_categories;
};

#endif // APPCHOOSERCATEGORIES_H
