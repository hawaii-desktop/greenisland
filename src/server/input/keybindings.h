/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2016 Pier Luigi Fiorini
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
