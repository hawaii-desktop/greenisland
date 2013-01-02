/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2010 Canonical, Ltd.
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

#ifndef LISTAGGREGATORMODEL_H
#define LISTAGGREGATORMODEL_H

#include <QAbstractListModel>

/* Aggregates the data of several models and present them to the client
   as if they were one single model.
   The models that can be aggregated can only be QAbstractListModels or
   QSortFilterProxyModels.
   For this reason there are several non public methods in this class
   that handle QAbstractItemModel (which is the common ancestor of the
   two classes we can aggregate), but please note that this is only for
   keeping the code simpler.
   The public interface checks that the models it manipulates are of the
   accepted types only.
*/
class ListAggregatorModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ListAggregatorModel(QObject *parent = 0);
    ~ListAggregatorModel();

    // Allow test fixtures to access protected and private members
    friend class ListAggregatorModelTest;

    QHash<int, QByteArray> roleNames() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE QVariant get(int row) const;
    Q_INVOKABLE virtual bool removeRows(int row, int count,
                                        const QModelIndex &parent = QModelIndex());

    /* This method is the QML equivalent of aggregateListModel.
       The reason why aggregateListModel wasn't directly exposed to QML is that
       QAbstractListModel can't be exposed to QML directly since it's an abstract
       class. Therefore we accept a QVariant here and internally cast it back
       to a QAbstractListModel. We also accept QSortFilterProxyModels. */
    Q_INVOKABLE void appendModel(const QVariant &model);

public slots:
    /* Move one item from one position to another position.
       The item must remain in the same model. */
    void move(int from, int to);

protected:
    QList<QAbstractItemModel *> m_models;

    void aggregateListModel(QAbstractItemModel *model);
    void removeListModel(QAbstractItemModel *model);

private slots:
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onRowsRemoved(const QModelIndex &parent, int first, int last);
    void onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);

private:
    int computeOffset(QAbstractItemModel *model) const;
    QAbstractItemModel *modelAtIndex(int index) const;
};

#endif // LISTAGGREGATORMODEL_H
