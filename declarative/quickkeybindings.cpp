/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#include "quickkeybindings.h"

QuickKeyBindings::QuickKeyBindings(QObject *parent)
    : KeyBindings(parent)
{
}

QQmlListProperty<KeyBinding> QuickKeyBindings::keyBindingsList()
{
    return QQmlListProperty<KeyBinding>(this, this,
                                        QuickKeyBindings::keyBindingsCount,
                                        QuickKeyBindings::keyBindingsAt);
}

int QuickKeyBindings::keyBindingsCount(QQmlListProperty<KeyBinding> *list)
{
    return static_cast<QuickKeyBindings *>(list->data)->keyBindings().size();
}

KeyBinding *QuickKeyBindings::keyBindingsAt(QQmlListProperty<KeyBinding> *list, int index)
{
    return static_cast<QuickKeyBindings *>(list->data)->keyBindings().at(index);
}

#include "moc_quickkeybindings.cpp"
