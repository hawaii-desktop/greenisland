/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
