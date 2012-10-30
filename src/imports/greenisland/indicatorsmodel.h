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

#ifndef INDICATORSMODEL_H
#define INDICATORSMODEL_H

#include <QAbstractListModel>

class VIndicator;
class VIndicatorPlugin;

class IndicatorsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum {
        IndicatorRole = Qt::UserRole + 1
    };

    explicit IndicatorsModel(QObject *parent = 0);

    QHash<int, QByteArray> roleNames() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex indexForName(const QString &name) const;

private:
    Q_DISABLE_COPY(IndicatorsModel);

    QList<VIndicator *> m_indicators;

    void loadPlugin(const QString &name, VIndicatorPlugin *plugin);
};

#endif // INDICATORSMODEL_H
