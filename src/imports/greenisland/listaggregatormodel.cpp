/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 * Copyright (c) 2010 Canonical, Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QDebug>
#include <QSortFilterProxyModel>

#include "listaggregatormodel.h"

ListAggregatorModel::ListAggregatorModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ListAggregatorModel::~ListAggregatorModel()
{
}

QHash<int, QByteArray> ListAggregatorModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[0] = "item";
    return roles;
}

void ListAggregatorModel::appendModel(const QVariant &model)
{
    if (!model.isValid()) {
        qWarning() << "Unable to append a model that is not of type QAbstractListModel.";
        return;
    }
    QObject *object = qvariant_cast<QObject *>(model);
    if (!object) {
        qWarning() << "Unable to append a model that is not of type QAbstractListModel, "
                   << model << "is of type" << model.typeName();
        return;
    }
    QAbstractItemModel *list = qobject_cast<QAbstractListModel *>(object);
    if (!list) {
        list = qobject_cast<QSortFilterProxyModel *>(object);
        if (!list) {
            qWarning() << "Unable to append a model that is not of type QAbstractListModel, "
                       << object->objectName() << "is of type"
                       << object->metaObject()->className();
            return;
        }
    }
    aggregateListModel(list);
}

void ListAggregatorModel::aggregateListModel(QAbstractItemModel *model)
{
    if (!model)
        return;

    int modelRowCount = model->rowCount();
    if (modelRowCount > 0) {
        int first = rowCount();
        int last = first + modelRowCount - 1;
        beginInsertRows(QModelIndex(), first, last);
    }

    m_models.append(model);
    if (modelRowCount > 0)
        endInsertRows();

    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            SLOT(onRowsInserted(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            SLOT(onRowsRemoved(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            SLOT(onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
}

void ListAggregatorModel::removeListModel(QAbstractItemModel *model)
{
    int modelRowCount = model->rowCount();
    if (modelRowCount > 0) {
        int first = computeOffset(model);
        int last = first + modelRowCount - 1;
        beginRemoveRows(QModelIndex(), first, last);
    }

    m_models.removeOne(model);
    if (modelRowCount > 0)
        endRemoveRows();

    disconnect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
               this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
    disconnect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
               this, SLOT(onRowsRemoved(const QModelIndex &, int, int)));
    disconnect(model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this, SLOT(onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
}

void ListAggregatorModel::move(int from, int to)
{
    QAbstractItemModel *model = modelAtIndex(from);
    if (modelAtIndex(to) != model) {
        qWarning() << "cannot move an item from one model to another";
        return;
    }

    if (qobject_cast<QSortFilterProxyModel *>(model) != NULL) {
        qWarning() << "cannot move the items of a QSortFilterProxyModel";
        return;
    }

    int offset = computeOffset(model);
    // "move" is not a member of QAbstractItemModel, cannot be invoked directly
    QMetaObject::invokeMethod(model, "move",
                              Q_ARG(int, from - offset),
                              Q_ARG(int, to - offset));
}

int ListAggregatorModel::computeOffset(QAbstractItemModel *model) const
{
    int offset = 0;
    QList<QAbstractItemModel *>::const_iterator iter;
    for (iter = m_models.begin(); (iter != m_models.end()) && (*iter != model); ++iter)
        offset += (*iter)->rowCount();
    return offset;
}

QAbstractItemModel *ListAggregatorModel::modelAtIndex(int index) const
{
    int offset = index;
    foreach(QAbstractItemModel * model, m_models) {
        int size = model->rowCount();
        if (offset < size)
            return model;
        offset -= size;
    }
    return 0;
}

void ListAggregatorModel::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    QAbstractListModel *model = static_cast<QAbstractListModel *>(sender());
    int offset = computeOffset(model);
    beginInsertRows(parent, first + offset, last + offset);
    endInsertRows();
}

void ListAggregatorModel::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    QAbstractListModel *model = static_cast<QAbstractListModel *>(sender());
    int offset = computeOffset(model);
    beginRemoveRows(parent, first + offset, last + offset);
    endRemoveRows();
}

void ListAggregatorModel::onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                      const QModelIndex &destinationParent, int destinationRow)
{
    QAbstractListModel *model = static_cast<QAbstractListModel *>(sender());
    int offset = computeOffset(model);
    beginMoveRows(sourceParent, sourceStart + offset, sourceEnd + offset,
                  destinationParent, destinationRow + offset);
    endMoveRows();
}

int ListAggregatorModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    int count = 0;
    QList<QAbstractItemModel *>::const_iterator iter;
    for (iter = m_models.begin(); iter != m_models.end(); ++iter)
        count += (*iter)->rowCount();
    return count;
}

QVariant ListAggregatorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int offset = row;
    QList<QAbstractItemModel *>::const_iterator iter;
    for (iter = m_models.begin(); iter != m_models.end(); ++iter) {
        int rowCount = (*iter)->rowCount();
        if (offset >= rowCount) {
            offset -= rowCount;
        } else {
            QModelIndex new_index = (*iter)->index(offset, 0);
            return (*iter)->data(new_index, role);
        }
    }

    // For the sake of completeness, should never happen.
    return QVariant();
}

QVariant ListAggregatorModel::get(int row) const
{
    return data(QAbstractListModel::index(row), 0);
}

bool ListAggregatorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row >= rowCount() || count <= 0)
        return false;

    /* Note that since this is an aggregator, the underlying models will
       take care of signaling the addition and removal of rows and this
       model will update accordingly.
       Therefore we don't need to call beginRemoveRows */

    int removed = 0;
    foreach(QAbstractItemModel * model, m_models) {
        /* Please note that the offset of the current model is computed
           by iterating over all previous models and making a sum of their
           row count.
           By taking that into account the calculation for removeAt is:
           (row + removed) - (offset + removed)
           This can be simplified to just row - offset as you see below */
        int offset = computeOffset(model);
        int removeAt = row - offset;

        if (removeAt < 0 || removeAt >= model->rowCount()) {
            // The item(s) that we need to remove are outside of this model
            // move on to the next one.
            continue;
        }

        int removeCount = qMin(count - removed, model->rowCount() - removeAt);
        model->removeRows(removeAt, removeCount);
        removed += removeCount;

        if (removed >= count)
            break;
    }

    return true;
}

#include "moc_listaggregatormodel.cpp"
