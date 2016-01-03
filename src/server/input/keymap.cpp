/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "keymap.h"
#include "keymap_p.h"

namespace GreenIsland {

namespace Server {

Keymap::Keymap(QObject *parent)
    : QObject(*new KeymapPrivate(), parent)
{
}

QString Keymap::layout() const
{
    Q_D(const Keymap);
    return d->layout;
}

void Keymap::setLayout(const QString &layout)
{
    Q_D(Keymap);

    if (d->layout == layout)
        return;

    d->layout = layout;
    Q_EMIT layoutChanged();
}

QString Keymap::variant() const
{
    Q_D(const Keymap);
    return d->variant;
}

void Keymap::setVariant(const QString &variant)
{
    Q_D(Keymap);

    if (d->variant == variant)
        return;

    d->variant = variant;
    Q_EMIT variantChanged();
}

QString Keymap::options() const
{
    Q_D(const Keymap);
    return d->options;
}

void Keymap::setOptions(const QString &options)
{
    Q_D(Keymap);

    if (d->options == options)
        return;

    d->options = options;
    Q_EMIT optionsChanged();
}

QString Keymap::model() const
{
    Q_D(const Keymap);
    return d->model;
}

void Keymap::setModel(const QString &model)
{
    Q_D(Keymap);

    if (d->model == model)
        return;

    d->model = model;
    Q_EMIT modelChanged();
}

QString Keymap::rules() const
{
    Q_D(const Keymap);
    return d->rules;
}

void Keymap::setRules(const QString &rules)
{
    Q_D(Keymap);

    if (d->rules == rules)
        return;

    d->rules = rules;
    Q_EMIT rulesChanged();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_keymap.cpp"
