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

#include "screen.h"

/*
 * ScreenPrivate
 */

class ScreenPrivate
{
public:
    ScreenPrivate();

    QString name;
    bool primary;
    QRect geometry;
};

ScreenPrivate::ScreenPrivate()
    : primary(false)
{
}

/*
 * Screen
 */

Screen::Screen(QObject *parent)
    : QObject(parent)
    , d_ptr(new ScreenPrivate())
{
}

Screen::~Screen()
{
    delete d_ptr;
}

QString Screen::name() const
{
    Q_D(const Screen);
    return d->name;
}

void Screen::setName(const QString &name)
{
    Q_D(Screen);

    if (d->name != name && !name.isEmpty()) {
        d->name = name;
        Q_EMIT nameChanged();
    }
}

bool Screen::isPrimary() const
{
    Q_D(const Screen);
    return d->primary;
}

void Screen::setPrimary(bool value)
{
    Q_D(Screen);

    if (d->primary != value) {
        d->primary = value;
        Q_EMIT primaryChanged();
    }
}

QRect Screen::geometry() const
{
    Q_D(const Screen);
    return d->geometry;
}

void Screen::setGeometry(const QRect &rect)
{
    Q_D(Screen);

    if (d->geometry != rect) {
        d->geometry = rect;
        Q_EMIT geometryChanged();
    }
}

#include "moc_screen.cpp"
