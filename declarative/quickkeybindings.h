/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
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

#ifndef QUICKKEYBINDINGS_H
#define QUICKKEYBINDINGS_H

#include <QtQml/QQmlListProperty>

#include <GreenIsland/Server/KeyBindings>

using namespace GreenIsland::Server;

class QuickKeyBindings : public KeyBindings
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<KeyBinding> keyBindings READ keyBindingsList DESIGNABLE false)
public:
    QuickKeyBindings(QObject *parent = Q_NULLPTR);

    QQmlListProperty<KeyBinding> keyBindingsList();

    static int keyBindingsCount(QQmlListProperty<KeyBinding> *list);
    static KeyBinding *keyBindingsAt(QQmlListProperty<KeyBinding> *list, int index);
};

#endif // QUICKKEYBINDINGS_H
