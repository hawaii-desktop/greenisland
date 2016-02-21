/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini
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

#include <QtQuick/QQuickWindow>

#include "keybindingsfilter.h"

KeyBindingsFilter::KeyBindingsFilter(QQuickItem *parent)
    : QQuickItem(parent)
{
    connect(this, &QQuickItem::windowChanged, this, [=](QQuickWindow *window) {
        // Remove event filter previously installed if any
        if (!m_window.isNull()) {
            m_window->removeEventFilter(this);
            m_window.clear();
        }

        // Install this event filter when the item is on the window
        if (window) {
            m_window = window;
            window->installEventFilter(this);
        }
    });
}

QuickKeyBindings *KeyBindingsFilter::keyBindings() const
{
    return m_keyBindings.data();
}

void KeyBindingsFilter::setKeyBindings(QuickKeyBindings *keyBindings)
{
    if (m_keyBindings.data() == keyBindings)
        return;

    m_keyBindings = keyBindings;
    Q_EMIT keyBindingsChanged();
}

bool KeyBindingsFilter::eventFilter(QObject *object, QEvent *event)
{
    // Only filter the window we are attached to
    if (object != m_window.data())
        return QObject::eventFilter(object, event);

    // Discard events not related to keyboard
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return false;

    // Do not continue if no keybindings manager is set
    if (m_keyBindings.isNull())
        return false;

    QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

    // Catch a keybinding being pressed or released and eat the event,
    // the application window currently focused will never receive it
    Q_FOREACH (KeyBinding *keyBinding, m_keyBindings.data()->keyBindings()) {
        if (keyBinding->matches(keyEvent->key(), keyEvent->modifiers())) {
            if (event->type() == QEvent::KeyPress)
                Q_EMIT m_keyBindings.data()->keyBindingPressed(keyBinding->name());
            else
                Q_EMIT m_keyBindings.data()->keyBindingReleased(keyBinding->name());
            Q_EMIT m_keyBindings.data()->keyBindingTriggered(
                        event->type() == QEvent::KeyPress ? KeyBindings::Press : KeyBindings::Release,
                        keyBinding->name());

            return true;
        }
    }

    return false;
}

#include "moc_keybindingsfilter.cpp"
