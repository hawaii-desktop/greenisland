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

#include <QGuiApplication>
#include <QQuickView>
#include <QQuickItem>

#include "vindicator.h"
#include "vindicator_p.h"

/*
 * VIndicatorPrivate
 */

VIndicatorPrivate::VIndicatorPrivate()
{
}

/*
 * VIndicator
 */

VIndicator::VIndicator(const QString &name)
    : QObject()
    , d_ptr(new VIndicatorPrivate())
{
    d_ptr->name = name;
}

QString VIndicator::name() const
{
    Q_D(const VIndicator);
    return d->name;
}

QString VIndicator::label() const
{
    Q_D(const VIndicator);
    return d->label;
}

QString VIndicator::iconName() const
{
    Q_D(const VIndicator);
    return d->iconName;
}

QQuickItem *VIndicator::item() const
{
    return 0;
}

void VIndicator::setLabel(const QString &label)
{
    Q_D(VIndicator);

    if (d->label != label) {
        d->label = label;
        emit labelChanged();
    }
}

void VIndicator::setIconName(const QString &iconName)
{
    Q_D(VIndicator);

    if (d->iconName != iconName) {
        d->iconName = iconName;
        emit iconNameChanged();
    }
}

#include "moc_vindicator.cpp"
