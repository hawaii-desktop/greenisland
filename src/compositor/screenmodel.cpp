/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
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

#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/Screen>

#include "screenmodel.h"

static bool outputLess(KScreen::Output *a, KScreen::Output *b)
{
    return ((a->isEnabled() && !b->isEnabled())
            || (a->isEnabled() == b->isEnabled() && (a->isPrimary() && !b->isPrimary()))
            || (a->isPrimary() == b->isPrimary() && (a->pos().x() < b->pos().x()
                                                     || (a->pos().x() == b->pos().x() && a->pos().y() < b->pos().y()))));
}

/*
 * ScreenModelPrivate
 */

class ScreenModelPrivate
{
public:
    ScreenModelPrivate(ScreenModel *self);

    KScreen::Config *config;
    QList<KScreen::Output *> outputs;
    QRect totalGeometry;

    QList<KScreen::Output*> sortOutputs(const QHash<int, KScreen::Output*> &outputs) const;

    void addOutput(KScreen::Output *output);
    void removeOutput(KScreen::Output *output);

    void _q_screenSizeChanged();
    void _q_outputAdded(KScreen::Output *output);
    void _q_outputRemoved(int id);
    void _q_outputEnabledChanged();
    void _q_primaryChanged();
    void _q_rotationChanged();
    void _q_geometryChanged();

private:
    Q_DECLARE_PUBLIC(ScreenModel)
    ScreenModel *q_ptr;
};

ScreenModelPrivate::ScreenModelPrivate(ScreenModel *self)
    : q_ptr(self)
{
    // Load current configuration
    config = KScreen::Config::current();
}

QList<KScreen::Output *> ScreenModelPrivate::sortOutputs(const QHash<int, KScreen::Output *> &outputs) const
{
    QList<KScreen::Output *> ret = outputs.values();
    std::sort(ret.begin(), ret.end(), outputLess);
    return ret;
}

void ScreenModelPrivate::addOutput(KScreen::Output *output)
{
    Q_Q(ScreenModel);

    q->connect(output, SIGNAL(isEnabledChanged()),
               q, SLOT(_q_outputEnabledChanged()),
               Qt::UniqueConnection);
    q->connect(output, SIGNAL(isPrimaryChanged()),
               q, SLOT(_q_primaryChanged()),
               Qt::UniqueConnection);
    q->connect(output, SIGNAL(posChanged()),
               q, SLOT(_q_geometryChanged()),
               Qt::UniqueConnection);
    q->connect(output, SIGNAL(currentModeIdChanged()),
               q, SLOT(_q_geometryChanged()));

    q->beginInsertRows(QModelIndex(), outputs.size(), outputs.size());
    outputs.append(output);
    q->endInsertRows();
}

void ScreenModelPrivate::removeOutput(KScreen::Output *output)
{
    Q_Q(ScreenModel);

    int row = outputs.indexOf(output);

    q->beginRemoveRows(QModelIndex(), row, row);
    outputs.removeOne(output);
    q->endRemoveRows();
}

void ScreenModelPrivate::_q_screenSizeChanged()
{
    Q_Q(ScreenModel);

    totalGeometry = QRect(QPoint(0, 0), config->screen()->currentSize());
    Q_EMIT q->totalGeometryChanged();
}

void ScreenModelPrivate::_q_outputAdded(KScreen::Output *output)
{
    addOutput(output);
}

void ScreenModelPrivate::_q_outputRemoved(int id)
{
    for (KScreen::Output *output: outputs) {
        if (output->id() == id) {
            removeOutput(output);
            return;
        }
    }
}

void ScreenModelPrivate::_q_outputEnabledChanged()
{
    Q_Q(ScreenModel);

    KScreen::Output *output = qobject_cast<KScreen::Output *>(q->sender());

    if (output->isEnabled())
        addOutput(output);
    else
        removeOutput(output);
}

void ScreenModelPrivate::_q_primaryChanged()
{
    Q_Q(ScreenModel);

    KScreen::Output *output = qobject_cast<KScreen::Output *>(q->sender());

    if (outputs.contains(output)) {
        int row = outputs.indexOf(output);
        QModelIndex index = q->index(row);
        Q_EMIT q->dataChanged(index, index, QVector<int>() << ScreenModel::PrimaryRole);
    }
}

void ScreenModelPrivate::_q_rotationChanged()
{
    Q_Q(ScreenModel);

    KScreen::Output *output = qobject_cast<KScreen::Output *>(q->sender());

    if (outputs.contains(output)) {
        int row = outputs.indexOf(output);
        QModelIndex index = q->index(row);
        Q_EMIT q->dataChanged(index, index, QVector<int>() << ScreenModel::RotationRole);
    }
}

void ScreenModelPrivate::_q_geometryChanged()
{
    Q_Q(ScreenModel);

    KScreen::Output *output = qobject_cast<KScreen::Output *>(q->sender());

    if (outputs.contains(output)) {
        int row = outputs.indexOf(output);
        QModelIndex index = q->index(row);
        Q_EMIT q->dataChanged(index, index, QVector<int>() << ScreenModel::GeometryRole);
    }
}

/*
 * ScreenModel
 */

ScreenModel::ScreenModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new ScreenModelPrivate(this))
{
    Q_D(ScreenModel);

    // Add configuration to the monitor
    KScreen::ConfigMonitor::instance()->addConfig(d->config);

    // Connect signals
    connect(d->config->screen(), SIGNAL(currentSizeChanged()),
            this, SLOT(_q_screenSizeChanged()));
    connect(d->config, SIGNAL(outputAdded(KScreen::Output*)),
            this, SLOT(_q_outputAdded(KScreen::Output*)));
    connect(d->config, SIGNAL(outputRemoved(int)),
            this, SLOT(_q_outputRemoved(int)));

    // Initialize model
    for (KScreen::Output *output: d->sortOutputs(d->config->outputs()))
        d->addOutput(output);
    d->_q_screenSizeChanged();
}

ScreenModel::~ScreenModel()
{
    delete d_ptr;
}

QRect ScreenModel::totalGeometry() const
{
    Q_D(const ScreenModel);
    return d->totalGeometry;
}

QHash<int, QByteArray> ScreenModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[NumberRole] = "number";
    roles[PrimaryRole] = "primary";
    roles[RotationRole] = "rotation";
    roles[GeometryRole] = "geometry";
    return roles;
}

int ScreenModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_D(const ScreenModel);

    return d->outputs.size();
}

QVariant ScreenModel::data(const QModelIndex &index, int role) const
{
    Q_D(const ScreenModel);

    if (!index.isValid() || index.row() >= d->outputs.size())
        return QVariant();

    KScreen::Output *output = d->outputs.at(index.row());

    switch (role) {
    case NameRole:
        return output->name();
    case NumberRole:
        return output->id();
    case PrimaryRole:
        return output->isPrimary();
    case RotationRole:
        switch (output->rotation()) {
        case KScreen::Output::None:
            return qreal(0);
        case KScreen::Output::Left:
            return qreal(270);
        case KScreen::Output::Inverted:
            return qreal(180);
        case KScreen::Output::Right:
            return qreal(90);
        }
    case GeometryRole:
        switch (output->rotation()) {
        case KScreen::Output::None:
        case KScreen::Output::Inverted:
            return output->geometry();
        case KScreen::Output::Left:
        case KScreen::Output::Right:
            return QRect(output->pos(),
                         QSize(output->geometry().size().height(), output->geometry().size().width()));
        }
    default:
        break;
    }

    return QVariant();
}

#include "moc_screenmodel.cpp"
