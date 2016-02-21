/****************************************************************************
**
** Copyright (C) 2015-2016 Pier Luigi Fiorini
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <QtCore/private/qobject_p.h>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include "logging.h"
#include "deviceintegration/eglfsxkb.h"
#include "libinput/libinputhandler.h"
#include "libinput/libinputkeyboard.h"

#include <libinput.h>

namespace GreenIsland {

namespace Platform {

/*
 * LibInputKeyboardPrivate
 */

class LibInputKeyboardPrivate : public QObjectPrivate
{
public:
    LibInputKeyboardPrivate(LibInputHandler *h)
        : handler(h)
        , context(Q_NULLPTR)
        , keymap(Q_NULLPTR)
        , state(Q_NULLPTR)
        , repeatRate(40)
        , repeatDelay(400)
    {
        context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        if (!context) {
            qCWarning(lcInput) << "Unable to create xkb context";
            return;
        }

        keymap = xkb_keymap_new_from_names(context, Q_NULLPTR,
                                           XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (!keymap) {
            qCWarning(lcInput) << "Unable to compile xkb keymap";
            xkb_context_unref(context);
            context = Q_NULLPTR;
            return;
        }

        state = xkb_state_new(keymap);
        if (!state) {
            qCWarning(lcInput) << "Unable to create xkb state";
            xkb_keymap_unref(keymap);
            keymap = Q_NULLPTR;
            xkb_context_unref(context);
            context = Q_NULLPTR;
            return;
        }

        modifiers[0] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
        modifiers[1] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
        modifiers[2] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
        modifiers[3] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_LOGO);

        repeatTimer.setSingleShot(true);
    }

    ~LibInputKeyboardPrivate()
    {
        if (state)
            xkb_state_unref(state);
        if (keymap)
            xkb_keymap_unref(keymap);
        if (context)
            xkb_context_unref(context);
    }

    void _q_handleRepeat()
    {
        LibInputKeyEvent keyEvent;
        keyEvent.key = repeatData.key;
        keyEvent.modifiers = repeatData.modifiers;
        keyEvent.nativeScanCode = repeatData.nativeScanCode;
        keyEvent.nativeVirtualKey = repeatData.nativeVirtualKey;
        keyEvent.nativeModifiers = repeatData.nativeModifiers;
        keyEvent.text = repeatData.text;
        keyEvent.autoRepeat = true;
        keyEvent.repeatCount = repeatData.repeatCount;
        Q_EMIT handler->keyPressed(keyEvent);

        ++repeatData.repeatCount;
        repeatTimer.setInterval(repeatRate);
        repeatTimer.start();
    }

    LibInputHandler *handler;

    xkb_context *context;
    xkb_keymap *keymap;
    xkb_state *state;
    xkb_mod_index_t modifiers[4];

    QTimer repeatTimer;
    quint32 repeatRate;
    quint32 repeatDelay;
    struct {
        int key;
        Qt::KeyboardModifiers modifiers;
        quint32 nativeScanCode;
        quint32 nativeVirtualKey;
        quint32 nativeModifiers;
        QString text;
        ushort repeatCount;
    } repeatData;
};

/*
 * LibInputKeyboard
 */

LibInputKeyboard::LibInputKeyboard(LibInputHandler *handler, QObject *parent)
    : QObject(*new LibInputKeyboardPrivate(handler), parent)
{
    Q_D(LibInputKeyboard);
    connect(&d->repeatTimer, SIGNAL(timeout()),
            this, SLOT(_q_handleRepeat()));
}

void LibInputKeyboard::handleKey(libinput_event_keyboard *event)
{
    Q_D(LibInputKeyboard);

    if (!d->context || !d->keymap || !d->state)
        return;

    const quint32 key = libinput_event_keyboard_get_key(event) + 8;
    const xkb_keysym_t keysym = xkb_state_key_get_one_sym(d->state, key);
    const bool isPressed = libinput_event_keyboard_get_key_state(event) == LIBINPUT_KEY_STATE_PRESSED;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    // Text
    QVarLengthArray<char, 32> chars(32);
    const int size = xkb_state_key_get_utf8(d->state, key, chars.data(), chars.size());
    if (Q_UNLIKELY(size + 1 > chars.size())) { // +1 for NUL
        chars.resize(size + 1);
        xkb_state_key_get_utf8(d->state, key, chars.data(), chars.size());
    }
    const QString text = QString::fromUtf8(chars.constData(), size);

    // Map keysym to Qt key
    const int qtkey = EglFSXkb::keysymToQtKey(keysym, modifiers, text);

    // Modifiers
    xkb_state_component type = xkb_state_component(XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);
    if (xkb_state_mod_index_is_active(d->state, d->modifiers[0], type) && (qtkey != Qt::Key_Control || !isPressed))
        modifiers |= Qt::ControlModifier;
    if (xkb_state_mod_index_is_active(d->state, d->modifiers[1], type) && (qtkey != Qt::Key_Alt || !isPressed))
        modifiers |= Qt::AltModifier;
    if (xkb_state_mod_index_is_active(d->state, d->modifiers[2], type) && (qtkey != Qt::Key_Shift || !isPressed))
        modifiers |= Qt::ShiftModifier;
    if (xkb_state_mod_index_is_active(d->state, d->modifiers[3], type) && (qtkey != Qt::Key_Meta || !isPressed))
        modifiers |= Qt::MetaModifier;

    xkb_state_update_key(d->state, key, isPressed ? XKB_KEY_DOWN : XKB_KEY_UP);

    // Event
    LibInputKeyEvent keyEvent;
    keyEvent.key = qtkey;
    keyEvent.modifiers = modifiers;
    keyEvent.nativeScanCode = key;
    keyEvent.nativeVirtualKey = keysym;
    keyEvent.nativeModifiers = modifiers;
    keyEvent.text = text;
    keyEvent.autoRepeat = false;
    keyEvent.repeatCount = 1;
    if (isPressed)
        Q_EMIT d->handler->keyPressed(keyEvent);
    else
        Q_EMIT d->handler->keyReleased(keyEvent);

    // Repeat
    if (isPressed && xkb_keymap_key_repeats(d->keymap, key)) {
        d->repeatData.key = qtkey;
        d->repeatData.modifiers = modifiers;
        d->repeatData.nativeScanCode = key;
        d->repeatData.nativeVirtualKey = keysym;
        d->repeatData.nativeModifiers = modifiers;
        d->repeatData.text = text;
        d->repeatData.repeatCount = 1;
        d->repeatTimer.setInterval(d->repeatDelay);
        d->repeatTimer.start();
    } else if (d->repeatTimer.isActive()) {
        d->repeatTimer.stop();
    }
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_libinputkeyboard.cpp"
