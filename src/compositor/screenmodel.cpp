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

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include "screenmodel.h"

/*
 * ScreenModelPrivate
 */

class ScreenModelPrivate
{
public:
    ScreenModelPrivate(ScreenModel *self);

    QRect totalGeometry;

    void initialize(bool first);

    void _q_geometryChanged(const QRect &rect);
    void _q_screenDestroyed(QObject *object);

private:
    Q_DECLARE_PUBLIC(ScreenModel)
    ScreenModel *q_ptr;
};

ScreenModelPrivate::ScreenModelPrivate(ScreenModel *self)
    : q_ptr(self)
{
}

void ScreenModelPrivate::initialize(bool first)
{
    Q_Q(ScreenModel);

    q->beginResetModel();

    totalGeometry = QRect();

    for (QScreen *screen: QGuiApplication::screens()) {
        totalGeometry = totalGeometry.united(screen->geometry());

        if (first)
            q->connect(screen, SIGNAL(geometryChanged(QRect)),
                       q, SLOT(_q_geometryChanged(QRect)));
        q->connect(screen, SIGNAL(destroyed(QObject*)),
                   q, SLOT(_q_screenDestroyed(QObject*)));
    }

    q->endResetModel();

    Q_EMIT q->totalGeometryChanged();
}

void ScreenModelPrivate::_q_geometryChanged(const QRect &rect)
{
    Q_UNUSED(rect);
    initialize(false);
}

void ScreenModelPrivate::_q_screenDestroyed(QObject *object)
{
    Q_UNUSED(object);
    initialize(false);
}

/*
 * ScreenModel
 */

ScreenModel::ScreenModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new ScreenModelPrivate(this))
{
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
    roles[PrimaryRole] = "primary";
    roles[GeometryRole] = "geometry";
    return roles;
}

int ScreenModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return QGuiApplication::screens().size();
}

QVariant ScreenModel::data(const QModelIndex &index, int role) const
{
    QList<QScreen *> screens = QGuiApplication::screens();

    if (!index.isValid() || index.row() >= screens.size())
        return QVariant();

    QScreen *screen = screens.at(index.row());

    switch (role) {
    case NameRole:
        return screen->name();
    case PrimaryRole:
        return QGuiApplication::primaryScreen() == screen;
    case GeometryRole:
        return screen->geometry();
    default:
        break;
    }

    return QVariant();
}

#include "moc_screenmodel.cpp"
