/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

namespace GreenIsland {

class KeyBindingPrivate
{
public:
    KeyBindingPrivate(const QString &_name, const QKeySequence &_sequence)
        : name(_name)
        , sequence(_sequence)
    {
    }

    QString name;
    QKeySequence sequence;
};

KeyBinding::KeyBinding(const QString &name, const QKeySequence &sequence)
    : d_ptr(new KeyBindingPrivate(name, sequence))
{
}

KeyBinding::KeyBinding(const KeyBinding &other)
    : d_ptr(new KeyBindingPrivate(other.name(), other.sequence()))
{
}

KeyBinding::~KeyBinding()
{
    delete d_ptr;
}

QString KeyBinding::name() const
{
    return d_ptr->name;
}

QKeySequence KeyBinding::sequence() const
{
    return d_ptr->sequence;
}

bool KeyBinding::matches(int key, const Qt::KeyboardModifiers &modifiers) const
{
    Qt::Key qtKey = static_cast<Qt::Key>(key);
    if (qtKey == Qt::Key_unknown)
        return false;

    // Might be a single press of a modifier
    switch (key) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
        if (d_ptr->sequence == QKeySequence(key))
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
    if (d_ptr->sequence == QKeySequence(keyInt))
        return true;

    return false;
}

KeyBinding &KeyBinding::operator=(const KeyBinding &other)
{
    if (this != &other) {
        d_ptr->name = other.name();
        d_ptr->sequence = other.sequence();
    }

    return *this;
}

bool KeyBinding::operator==(const KeyBinding &other)
{
    return (d_ptr->name == other.name() && d_ptr->sequence == other.sequence());
}

}
