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

#ifndef LAUNCHERMODEL_H
#define LAUNCHERMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>

#include "launcheritem.h"

class QUrl;

class VSettings;

class LauncherController;
class LauncherApplication;

class LauncherModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum {
        IconRole = Qt::UserRole + 1,
        LabelRole = Qt::UserRole + 2
    };

    LauncherModel(QObject *parent = 0);
    ~LauncherModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    Q_INVOKABLE void pinApplication(const QString &path);
    Q_INVOKABLE void pinUrl(const QUrl &url);

private:
    VSettings *m_settings;
    QList<LauncherItem *> m_list;

private:
    friend class LauncherController;

    void addItem(LauncherApplication *item);
    LauncherApplication *findItem(const QString &id, const QString &ver);
};

QML_DECLARE_TYPE(LauncherModel)

#endif // LAUNCHERMODEL_H
