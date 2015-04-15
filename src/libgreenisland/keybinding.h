/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef KEYBINDING_H
#define KEYBINDING_H

#include <QtGui/QKeySequence>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class KeyBindingPrivate;

class GREENISLAND_EXPORT KeyBinding
{
public:
    explicit KeyBinding(const QString &name, const QKeySequence &sequence);
    KeyBinding(const KeyBinding &other);
    ~KeyBinding();

    QString name() const;
    QKeySequence sequence() const;

    bool matches(int key, const Qt::KeyboardModifiers &modifiers,
                 const QString &text) const;

    bool operator==(const KeyBinding &other);

private:
    KeyBindingPrivate *const d_ptr;
};

}

#endif // KEYBINDING_H
