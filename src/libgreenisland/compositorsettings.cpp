/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCompositor/QWaylandInputDevice>

#include "compositorsettings.h"

namespace GreenIsland {

class CompositorSettingsPrivate
{
public:
    CompositorSettingsPrivate()
    {
    }

    Compositor *compositor;
    QWaylandKeymap keymap;
};

CompositorSettings::CompositorSettings(Compositor *compositor)
    : QObject(compositor)
    , d_ptr(new CompositorSettingsPrivate())
{
    qRegisterMetaType<CompositorSettings *>("CompositorSettings*");

    Q_D(CompositorSettings);
    d->compositor = compositor;
}

CompositorSettings::~CompositorSettings()
{
    delete d_ptr;
}

QString CompositorSettings::keyboardLayout() const
{
    Q_D(const CompositorSettings);
    return d->keymap.layout();
}

void CompositorSettings::setKeyboardLayout(const QString &layout)
{
    Q_D(CompositorSettings);

    if (d->keymap.layout() == layout)
        return;

    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardVariant() const
{
    Q_D(const CompositorSettings);
    return d->keymap.variant();
}

void CompositorSettings::setKeyboardVariant(const QString &variant)
{
    Q_D(CompositorSettings);

    if (d->keymap.variant() == variant)
        return;

    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardOptions() const
{
    Q_D(const CompositorSettings);
    return d->keymap.options();
}

void CompositorSettings::setKeyboardOptions(const QString &options)
{
    Q_D(CompositorSettings);

    if (d->keymap.options() == options)
        return;

    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardRules() const
{
    Q_D(const CompositorSettings);
    return d->keymap.rules();
}

void CompositorSettings::setKeyboardRules(const QString &rules)
{
    Q_D(CompositorSettings);

    if (d->keymap.rules() == rules)
        return;

    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardModel() const
{
    Q_D(const CompositorSettings);
    return d->keymap.model();
}

void CompositorSettings::setKeyboardModel(const QString &model)
{
    Q_D(CompositorSettings);

    if (d->keymap.model() == model)
        return;

    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

}

#include "moc_compositorsettings.cpp"
