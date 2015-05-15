/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "keybinding.h"
#include "keybindings.h"

namespace GreenIsland {

class KeyBindingsPrivate
{
public:
    KeyBindingsPrivate()
    {
    }

    QList<KeyBinding> keyBindings;
};

class KeyBindingsSingleton : public KeyBindings {};
Q_GLOBAL_STATIC(KeyBindingsSingleton, s_keyBindings)

KeyBindings::KeyBindings(QObject *parent)
    : QObject(parent)
    , d_ptr(new KeyBindingsPrivate)
{
}

KeyBindings::~KeyBindings()
{
    delete d_ptr;
}

KeyBindings *KeyBindings::instance()
{
    return s_keyBindings();
}

QList<KeyBinding> KeyBindings::keyBindings() const
{
    Q_D(const KeyBindings);
    return d->keyBindings;
}

bool KeyBindings::registerKeyBinding(const QString &name, const QString &keys)
{
    Q_D(KeyBindings);

    // Do we already have this keybinding?
    Q_FOREACH (const KeyBinding &binding, d->keyBindings) {
        if (binding.name() == name)
            return false;
    }

    // Is the keybinding text valid?
    QKeySequence sequence(keys);
    if (!sequence.isEmpty()) {
        d->keyBindings.append(KeyBinding(name, sequence));
        return true;
    }

    return false;
}

bool KeyBindings::unregisterKeyBinding(const QString &name)
{
    Q_D(KeyBindings);

    Q_FOREACH (const KeyBinding &binding, d->keyBindings) {
        if (binding.name() == name) {
            d->keyBindings.removeOne(binding);
            return true;
        }
    }

    return false;
}

}

#include "moc_keybindings.cpp"
