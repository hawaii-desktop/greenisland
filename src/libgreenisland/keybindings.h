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

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include <QtCore/QObject>

#include <greenisland/greenisland_export.h>

class KeyBinding;

namespace GreenIsland {

class KeyBindingsPrivate;

class GREENISLAND_EXPORT KeyBindings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KeyBindings)
public:
    virtual ~KeyBindings();

    static KeyBindings *instance();

    QList<KeyBinding> keyBindings() const;

    Q_INVOKABLE bool registerKeyBinding(const QString &name, const QString &keys);
    Q_INVOKABLE bool unregisterKeyBinding(const QString &name);

Q_SIGNALS:
    void keyBindingPressed(const QString &name);
    void keyBindingReleased(const QString &name);

protected:
    KeyBindings(QObject *parent = 0);

private:
    KeyBindingsPrivate *const d_ptr;
};

}

#endif // KEYBINDINGS_H
