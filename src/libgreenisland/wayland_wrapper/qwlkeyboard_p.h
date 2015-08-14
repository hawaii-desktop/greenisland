/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTWAYLAND_QWLKEYBOARD_P_H
#define QTWAYLAND_QWLKEYBOARD_P_H

#include <greenisland/greenisland_export.h>
#include "inputdevice.h"

#include <QObject>
#include "qwayland-server-wayland.h"

#include <QtCore/QVector>

#ifdef HAVE_XKBCOMMON
#include <xkbcommon/xkbcommon.h>
#endif

#include "listener.h"


namespace GreenIsland {

class WlCompositor;
class WlInputDevice;
class WlSurface;
class WlKeyboard;

class GREENISLAND_EXPORT KeyboardGrabber {
    public:
        virtual ~KeyboardGrabber();
        virtual void focused(WlSurface *surface) = 0;
        virtual void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) = 0;
        virtual void modifiers(uint32_t serial, uint32_t mods_depressed,
                uint32_t mods_latched, uint32_t mods_locked, uint32_t group) = 0;

        WlKeyboard *m_keyboard;
};

class GREENISLAND_EXPORT WlKeyboard : public QObject, public QtWaylandServer::wl_keyboard, public KeyboardGrabber
{
    Q_OBJECT

public:
    WlKeyboard(WlCompositor *compositor, WlInputDevice *seat);
    ~WlKeyboard();

    void setFocus(WlSurface *surface);
    void setKeymap(const Keymap &keymap);

    void sendKeyModifiers(Resource *resource, uint32_t serial);
    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    WlSurface *focus() const;
    Resource *focusResource() const;

    void focused(WlSurface* surface);
    void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void modifiers(uint32_t serial, uint32_t mods_depressed,
                uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

    void keyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void updateKeymap();

   void startGrab(KeyboardGrabber *grab);
   void endGrab();
   KeyboardGrabber *currentGrab() const;

#ifdef HAVE_XKBCOMMON
    struct xkb_state *xkbState() const { return m_state; }
    uint32_t xkbModsMask() const { return m_modsDepressed | m_modsLatched | m_modsLocked; }
#endif

Q_SIGNALS:
    void focusChanged(WlSurface *surface);

protected:
    void keyboard_bind_resource(Resource *resource);
    void keyboard_destroy_resource(Resource *resource);
    void keyboard_release(Resource *resource) Q_DECL_OVERRIDE;

private:
    void sendKeyEvent(uint code, uint32_t state);
    void focusDestroyed();

#ifdef HAVE_XKBCOMMON
    void initXKB();
    void createXKBKeymap();
#endif

    WlCompositor *m_compositor;
    WlInputDevice *m_seat;

    KeyboardGrabber* m_grab;
    WlSurface *m_focus;
    Resource *m_focusResource;
    Listener m_focusDestroyListener;

    QVector<uint32_t> m_keys;
    uint32_t m_modsDepressed;
    uint32_t m_modsLatched;
    uint32_t m_modsLocked;
    uint32_t m_group;

    Keymap m_keymap;
    bool m_pendingKeymap;
#ifdef HAVE_XKBCOMMON
    size_t m_keymap_size;
    int m_keymap_fd;
    char *m_keymap_area;
    struct xkb_context *m_context;
    struct xkb_state *m_state;
#endif
};

} // namespace QtWayland


#endif // QTWAYLAND_QWLKEYBOARD_P_H
