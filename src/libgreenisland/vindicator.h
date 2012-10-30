/****************************************************************************
 * This file is part of libGreenIsland.
 *
 * Copyright (c) Pier Luigi Fiorini
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

#ifndef VINDICATOR_H
#define VINDICATOR_H

#include <QObject>

#include <VGreenIslandGlobal>

class QQuickView;

class VIndicatorPrivate;

class GREENISLAND_EXPORT VIndicator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString label READ label NOTIFY labelChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
public:
    explicit VIndicator(const QString &name = QString());

    QString name() const;
    QString label() const;
    QString iconName() const;

    Q_INVOKABLE virtual void showView();
    Q_INVOKABLE virtual void hideView();

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
