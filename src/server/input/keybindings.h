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

#ifndef GREENISLAND_KEYBINDINGS_H
#define GREENISLAND_KEYBINDINGS_H

#include <QtCore/QObject>
#include <QtGui/QKeySequence>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class KeyBindingPrivate;
class KeyBindingsPrivate;

class GREENISLANDSERVER_EXPORT KeyBinding : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KeyBinding)
    Q_DISABLE_COPY(KeyBinding)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PRIVATE_PROPERTY(d_func(), QString sequence READ sequenceString CONSTANT)
public:
    explicit KeyBinding(const QString &name, const QKeySequence &sequence,
                        QObject *parent = Q_NULLPTR);

    QString name() const;
    QKeySequence sequence() const;

    bool matches(int key, const Qt::KeyboardModifiers &modifiers) const;

    bool operator==(const KeyBinding &other);
};

class GREENISLANDSERVER_EXPORT KeyBindings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KeyBindings)
public:
    enum Action {
        Press = 1,
        Release
    };
    Q_ENUM(Action)

    KeyBindings(QObject *parent = Q_NULLPTR);

    QList<KeyBinding *> keyBindings() const;

    Q_INVOKABLE bool registerKeyBinding(const QString &name, const QString &keys);
    Q_INVOKABLE bool unregisterKeyBinding(const QString &name);

Q_SIGNALS:
    void keyBindingPressed(const QString &name);
    void keyBindingReleased(const QString &name);
    void keyBindingTriggered(Action action, const QString &name);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_KEYBINDINGS_H
