/****************************************************************************
 * This file is part of libGreenIsland.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef VINDICATOR_H
#define VINDICATOR_H

#include <QObject>

#include <VGreenIslandGlobal>

class QQuickView;
class QQuickItem;

class VIndicatorPrivate;

class GREENISLAND_EXPORT VIndicator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString label READ label NOTIFY labelChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(QQuickItem *item READ item)
public:
    explicit VIndicator(const QString &name = QString());

    QString name() const;
    QString label() const;
    QString iconName() const;
    virtual QQuickItem *item() const;

protected:
    void setLabel(const QString &label);
    void setIconName(const QString &iconName);

signals:
    void nameChanged();
    void labelChanged();
    void iconNameChanged();

private:
    Q_DECLARE_PRIVATE(VIndicator);
    VIndicatorPrivate *const d_ptr;
};

#endif // VINDICATOR_H
