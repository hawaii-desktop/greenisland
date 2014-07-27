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

#ifndef SCREENMODEL_H
#define SCREENMODEL_H

#include <QtCore/QAbstractListModel>

class ScreenModelPrivate;

namespace KScreen {
class Output;
}

class ScreenModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QRect totalGeometry READ totalGeometry NOTIFY totalGeometryChanged)
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PrimaryRole,
        RotationRole,
        GeometryRole
    };

    explicit ScreenModel(QObject *parent = 0);
    ~ScreenModel();

    QRect totalGeometry() const;

    QHash<int, QByteArray> roleNames() const;

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

Q_SIGNALS:
    void totalGeometryChanged();

private:
    Q_DECLARE_PRIVATE(ScreenModel)
    ScreenModelPrivate *const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_screenSizeChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_outputAdded(KScreen::Output *output))
    Q_PRIVATE_SLOT(d_func(), void _q_outputRemoved(int id))
    Q_PRIVATE_SLOT(d_func(), void _q_outputEnabledChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_rotationChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_primaryChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_geometryChanged())
};

#endif // SCREENMODEL_H
