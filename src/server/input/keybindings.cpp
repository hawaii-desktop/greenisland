/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2016 Pier Luigi Fiorini
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

#include "keybindings.h"
#include "keybindings_p.h"

namespace GreenIsland {

namespace Server {

/*
 * KeyBinding
 */

KeyBinding::KeyBinding(const QString &name, const QKeySequence &sequence, QObject *parent)
    : QObject(*new KeyBindingPrivate(name, sequence), parent)
{
}

QString KeyBinding::name() const
{
    Q_D(const KeyBinding);
    return d->name;
}

QKeySequence KeyBinding::sequence() const
{
    Q_D(const KeyBinding);
    return d->sequence;
}

bool KeyBinding::matches(int key, const Qt::KeyboardModifiers &modifiers) const
{
    Q_D(const KeyBinding);

    Qt::Key qtKey = static_cast<Qt::Key>(key);
    if (qtKey == Qt::Key_unknown)
        return false;

    // Might be a single press of a modifier
    switch (key) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
        if (d->sequence == QKeySequence(key))
            return true;
        break;
    default:
        break;
    }

    // Check for a combination of keys
    int keyInt = key;
    if (modifiers & Qt::ShiftModifier)
        keyInt += Qt::SHIFT;
    if (modifiers & Qt::ControlModifier)
        keyInt += Qt::CTRL;
    if (modifiers & Qt::AltModifier)
        keyInt += Qt::ALT;
    if (modifiers & Qt::MetaModifier)
        keyInt += Qt::META;
    if (d->sequence == QKeySequence(keyInt))
        return true;

    return false;
}

bool KeyBinding::operator==(const KeyBinding &other)
{
    Q_D(KeyBinding);
    return (d->name == other.name() && d->sequence == other.sequence());
}

/*
 * KeyBindings
 */

KeyBindings::KeyBindings(QObject *parent)
    : QObject(*new KeyBindingsPrivate(), parent)
{
}

QList<KeyBinding *> KeyBindings::keyBindings() const
{
    Q_D(const KeyBindings);
    return d->keyBindings;
}

bool KeyBindings::registerKeyBinding(const QString &name, const QString &keys)
{
    Q_D(KeyBindings);

    // Do we already have this keybinding?
    Q_FOREACH (KeyBinding *binding, d->keyBindings) {
        if (binding->name() == name)
            return false;
    }

    // Is the keybinding text valid?
    QKeySequence sequence(keys);
    if (!sequence.isEmpty()) {
        d->keyBindings.append(new KeyBinding(name, sequence));
        return true;
    }

    return false;
}

bool KeyBindings::unregisterKeyBinding(const QString &name)
{
    Q_D(KeyBindings);

    Q_FOREACH (KeyBinding *binding, d->keyBindings) {
        if (binding->name() == name) {
            d->keyBindings.removeOne(binding);
            return true;
        }
    }

    return false;
}

} // namespace Server

} // namespace GreenIsland

#include "moc_keybindings.cpp"
