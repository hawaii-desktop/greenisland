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

#include "compositorapp.h"
#include "screenmodel.h"

#include "fakescreenbackend.h"
#include "qscreenbackend.h"

/*
 * ScreenModelPrivate
 */

class ScreenModelPrivate
{
public:
    ScreenModelPrivate(ScreenModel *self);

    ScreenBackend *backend;
    QRect totalGeometry;

    void _q_screensChanged();

private:
    Q_DECLARE_PUBLIC(ScreenModel)
    ScreenModel *q_ptr;
};

ScreenModelPrivate::ScreenModelPrivate(ScreenModel *self)
    : q_ptr(self)
{
    CompositorApp *app = qobject_cast<CompositorApp *>(qApp);

    // Backend
    if (app->fakeScreenCount() > 0)
        backend = new FakeScreenBackend(self);
    else
        backend = new QScreenBackend(self);

    // Initialize
    _q_screensChanged();
}

void ScreenModelPrivate::_q_screensChanged()
{
    Q_Q(ScreenModel);

    totalGeometry = QRect();

    for (int i = 0; i < backend->count(); i++) {
        Screen *screen = backend->screenAt(i);
        totalGeometry = totalGeometry.united(screen->geometry());
    }

    Q_EMIT q->totalGeometryChanged();
}

/*
 * ScreenModel
 */

ScreenModel::ScreenModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new ScreenModelPrivate(this))
{
    Q_D(ScreenModel);

    connect(d->backend, SIGNAL(screenAdded()),
            this, SLOT(_q_screensChanged()));
    connect(d->backend, SIGNAL(screenChangedGeometry()),
            this, SLOT(_q_screensChanged()));
    connect(d->backend, SIGNAL(screenRemoved()),
            this, SLOT(_q_screensChanged()));
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
    Q_D(const ScreenModel);

    return d->backend->count();
}

QVariant ScreenModel::data(const QModelIndex &index, int role) const
{
    Q_D(const ScreenModel);

    if (!index.isValid() || index.row() >= d->backend->count())
        return QVariant();

    Screen *screen = d->backend->screenAt(index.row());

    switch (role) {
    case NameRole:
        return screen->name();
    case PrimaryRole:
        return screen->isPrimary();
    case GeometryRole:
        return screen->geometry();
    default:
        break;
    }

    return QVariant();
}

#include "moc_screenmodel.cpp"
