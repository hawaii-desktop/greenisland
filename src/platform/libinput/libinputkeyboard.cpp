/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <QtCore/private/qobject_p.h>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include "logging.h"
#include "libinput/libinputhandler.h"
#include "libinput/libinputkeyboard.h"

#include <xkbcommon/xkbcommon.h>

#include <libinput.h>

namespace GreenIsland {

namespace Platform {

struct KeyTableEntry {
    int xkb;
    int qt;
};

static inline bool operator==(const KeyTableEntry &a, const KeyTableEntry &b)
{
    return a.xkb == b.xkb;
}

static const KeyTableEntry keyTable[] = {
    { XKB_KEY_Escape,                  Qt::Key_Escape },
    { XKB_KEY_Tab,                     Qt::Key_Tab },
    { XKB_KEY_ISO_Left_Tab,            Qt::Key_Backtab },
    { XKB_KEY_BackSpace,               Qt::Key_Backspace },
    { XKB_KEY_Return,                  Qt::Key_Return },
    { XKB_KEY_Insert,                  Qt::Key_Insert },
    { XKB_KEY_Delete,                  Qt::Key_Delete },
    { XKB_KEY_Clear,                   Qt::Key_Delete },
    { XKB_KEY_Pause,                   Qt::Key_Pause },
    { XKB_KEY_Print,                   Qt::Key_Print },

    { XKB_KEY_Home,                    Qt::Key_Home },
    { XKB_KEY_End,                     Qt::Key_End },
    { XKB_KEY_Left,                    Qt::Key_Left },
    { XKB_KEY_Up,                      Qt::Key_Up },
    { XKB_KEY_Right,                   Qt::Key_Right },
    { XKB_KEY_Down,                    Qt::Key_Down },
    { XKB_KEY_Prior,                   Qt::Key_PageUp },
    { XKB_KEY_Next,                    Qt::Key_PageDown },

    { XKB_KEY_Shift_L,                 Qt::Key_Shift },
    { XKB_KEY_Shift_R,                 Qt::Key_Shift },
    { XKB_KEY_Shift_Lock,              Qt::Key_Shift },
    { XKB_KEY_Control_L,               Qt::Key_Control },
    { XKB_KEY_Control_R,               Qt::Key_Control },
    { XKB_KEY_Meta_L,                  Qt::Key_Meta },
    { XKB_KEY_Meta_R,                  Qt::Key_Meta },
    { XKB_KEY_Alt_L,                   Qt::Key_Alt },
    { XKB_KEY_Alt_R,                   Qt::Key_Alt },
    { XKB_KEY_Caps_Lock,               Qt::Key_CapsLock },
    { XKB_KEY_Num_Lock,                Qt::Key_NumLock },
    { XKB_KEY_Scroll_Lock,             Qt::Key_ScrollLock },
    { XKB_KEY_Super_L,                 Qt::Key_Super_L },
    { XKB_KEY_Super_R,                 Qt::Key_Super_R },
    { XKB_KEY_Menu,                    Qt::Key_Menu },
    { XKB_KEY_Hyper_L,                 Qt::Key_Hyper_L },
    { XKB_KEY_Hyper_R,                 Qt::Key_Hyper_R },
    { XKB_KEY_Help,                    Qt::Key_Help },

    { XKB_KEY_KP_Space,                Qt::Key_Space },
    { XKB_KEY_KP_Tab,                  Qt::Key_Tab },
    { XKB_KEY_KP_Enter,                Qt::Key_Enter },
    { XKB_KEY_KP_Home,                 Qt::Key_Home },
    { XKB_KEY_KP_Left,                 Qt::Key_Left },
    { XKB_KEY_KP_Up,                   Qt::Key_Up },
    { XKB_KEY_KP_Right,                Qt::Key_Right },
    { XKB_KEY_KP_Down,                 Qt::Key_Down },
    { XKB_KEY_KP_Prior,                Qt::Key_PageUp },
    { XKB_KEY_KP_Next,                 Qt::Key_PageDown },
    { XKB_KEY_KP_End,                  Qt::Key_End },
    { XKB_KEY_KP_Begin,                Qt::Key_Clear },
    { XKB_KEY_KP_Insert,               Qt::Key_Insert },
    { XKB_KEY_KP_Delete,               Qt::Key_Delete },
    { XKB_KEY_KP_Equal,                Qt::Key_Equal },
    { XKB_KEY_KP_Multiply,             Qt::Key_Asterisk },
    { XKB_KEY_KP_Add,                  Qt::Key_Plus },
    { XKB_KEY_KP_Separator,            Qt::Key_Comma },
    { XKB_KEY_KP_Subtract,             Qt::Key_Minus },
    { XKB_KEY_KP_Decimal,              Qt::Key_Period },
    { XKB_KEY_KP_Divide,               Qt::Key_Slash },

    { XKB_KEY_ISO_Level3_Shift,        Qt::Key_AltGr },
    { XKB_KEY_Multi_key,               Qt::Key_Multi_key },
    { XKB_KEY_Codeinput,               Qt::Key_Codeinput },
    { XKB_KEY_SingleCandidate,         Qt::Key_SingleCandidate },
    { XKB_KEY_MultipleCandidate,       Qt::Key_MultipleCandidate },
    { XKB_KEY_PreviousCandidate,       Qt::Key_PreviousCandidate },

    { XKB_KEY_Mode_switch,             Qt::Key_Mode_switch },
    { XKB_KEY_script_switch,           Qt::Key_Mode_switch },

    { XKB_KEY_XF86AudioPlay,           Qt::Key_MediaTogglePlayPause },
    { XKB_KEY_XF86AudioPause,          Qt::Key_MediaPause },
    { XKB_KEY_XF86AudioStop,           Qt::Key_MediaStop },
    { XKB_KEY_XF86AudioPrev,           Qt::Key_MediaPrevious },
    { XKB_KEY_XF86AudioNext,           Qt::Key_MediaNext },
    { XKB_KEY_XF86AudioRewind,         Qt::Key_MediaPrevious },
    { XKB_KEY_XF86AudioForward,        Qt::Key_MediaNext },
    { XKB_KEY_XF86AudioRecord,         Qt::Key_MediaRecord },

    { XKB_KEY_XF86AudioMute,           Qt::Key_VolumeMute },
    { XKB_KEY_XF86AudioLowerVolume,    Qt::Key_VolumeDown },
    { XKB_KEY_XF86AudioRaiseVolume,    Qt::Key_VolumeUp },

    { XKB_KEY_XF86AudioRandomPlay,     Qt::Key_AudioRandomPlay },
    { XKB_KEY_XF86AudioRepeat,         Qt::Key_AudioRepeat },

    { XKB_KEY_XF86ZoomIn,              Qt::Key_ZoomIn },
    { XKB_KEY_XF86ZoomOut,             Qt::Key_ZoomOut },

    { XKB_KEY_XF86Eject,               Qt::Key_Eject },

    { XKB_KEY_XF86Phone,               Qt::Key_ToggleCallHangup },
};

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

    int keysymToQtKey(xkb_keysym_t keysym) const
    {
        const size_t size = sizeof(keyTable) / sizeof(KeyTableEntry);
        KeyTableEntry entry;
        entry.xkb = keysym;
        const KeyTableEntry *result = std::find(keyTable, keyTable + size, entry);
        return result != keyTable + size ? result->qt : 0;
    }

    int keysymToQtKey(xkb_keysym_t keysym, Qt::KeyboardModifiers &modifiers, const QString &text) const
    {
        QTextCodec *codec = QTextCodec::codecForLocale();
        int code = 0;

        // Upper-case key
        if (keysym < 128 || (keysym < 256 && codec->mibEnum() == 4)) {
            code = ::isprint(int(keysym)) ? ::toupper(int(keysym)) : 0;
            // Function keys
        } else if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F35) {
            code = Qt::Key_F1 + (int(keysym) - XKB_KEY_F1);
            // Numeric keypad
        } else if (keysym >= XKB_KEY_KP_Space && keysym <= XKB_KEY_KP_9) {
            if (keysym >= XKB_KEY_KP_0)
                code = Qt::Key_0 + (int(keysym) - XKB_KEY_KP_0);
            else
                code = keysymToQtKey(keysym);
            modifiers |= Qt::KeypadModifier;
            // Unicode
        } else if (text.length() == 1 && text.unicode()->unicode() > 0x1f &&
                   text.unicode()->unicode() != 0x7f &&
                   !(keysym >= XKB_KEY_dead_grave && keysym <= XKB_KEY_dead_currency)) {
            code = text.unicode()->toUpper().unicode();
            // Any other key
        } else {
            code = keysymToQtKey(keysym);
        }

        return code;
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
    const int qtkey = d->keysymToQtKey(keysym, modifiers, text);

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
