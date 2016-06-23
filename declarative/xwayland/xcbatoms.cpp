/****************************************************************************
 * This file is part of Hawaii.
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

#include "xcbatoms.h"

namespace Xcb {

Atoms::Atoms()
    : wm_protocols(QByteArrayLiteral("WM_PROTOCOLS"), false)
    , wm_normal_hints(QByteArrayLiteral("WM_NORMAL_HINTS"), false)
    , wm_take_focus(QByteArrayLiteral("WM_TAKE_FOCUS"), false)
    , wm_delete_window(QByteArrayLiteral("WM_DELETE_WINDOW"), false)
    , wm_state(QByteArrayLiteral("WM_STATE"), false)
    , wm_s0(QByteArrayLiteral("WM_S0"), false)
    , wm_client_machine(QByteArrayLiteral("WM_CLIENT_MACHINE"), false)
    , net_wm_cm_s0(QByteArrayLiteral("_NET_WM_CM_S0"), false)
    , net_wm_name(QByteArrayLiteral("_NET_WM_NAME"), false)
    , net_wm_pid(QByteArrayLiteral("_NET_WM_PID"), false)
    , net_wm_icon(QByteArrayLiteral("_NET_WM_ICON"), false)
    , net_wm_state(QByteArrayLiteral("_NET_WM_STATE"), false)
    , net_wm_state_maximized_vert(QByteArrayLiteral("_NET_WM_STATE_MAXIMIZED_VERT"), false)
    , net_wm_state_maximized_horz(QByteArrayLiteral("_NET_WM_STATE_MAXIMIZED_HORZ"), false)
    , net_wm_state_fullscreen(QByteArrayLiteral("_NET_WM_STATE_FULLSCREEN"), false)
    , net_wm_user_time(QByteArrayLiteral("_NET_WM_USER_TIME"), false)
    , net_wm_icon_name(QByteArrayLiteral("_NET_WM_ICON_NAME"), false)
    , net_wm_desktop(QByteArrayLiteral("_NET_WM_DESKTOP"), false)
    , net_wm_window_type(QByteArrayLiteral("_NET_WM_WINDOW_TYPE"), false)
    , net_wm_window_type_desktop(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DESKTOP"), false)
    , net_wm_window_type_dock(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DOCK"), false)
    , net_wm_window_type_toolbar(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_TOOLBAR"), false)
    , net_wm_window_type_menu(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_MENU"), false)
    , net_wm_window_type_utility(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_UTILITY"), false)
    , net_wm_window_type_splash(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_SPLASH"), false)
    , net_wm_window_type_dialog(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DIALOG"), false)
    , net_wm_window_type_dropdown(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DROPDOWN_MENU"), false)
    , net_wm_window_type_popup(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_POPUP_MENU"), false)
    , net_wm_window_type_tooltip(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_TOOLTIP"), false)
    , net_wm_window_type_notification(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_NOTIFICATION"), false)
    , net_wm_window_type_combo(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_COMBO"), false)
    , net_wm_window_type_dnd(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DND"), false)
    , net_wm_window_type_normal(QByteArrayLiteral("_NET_WM_WINDOW_TYPE_NORMAL"), false)
    , net_wm_moveresize(QByteArrayLiteral("_NET_WM_MOVERESIZE"), false)
    , net_supporting_wm_check(QByteArrayLiteral("_NET_SUPPORTING_WM_CHECK"), false)
    , net_supported(QByteArrayLiteral("_NET_SUPPORTED"), false)
    , motif_wm_hints(QByteArrayLiteral("_MOTIF_WM_HINTS"), false)
    , clipboard(QByteArrayLiteral("CLIPBOARD"), false)
    , clipboard_manager(QByteArrayLiteral("CLIPBOARD_MANAGER"), false)
    , targets(QByteArrayLiteral("TARGETS"), false)
    , utf8_string(QByteArrayLiteral("UTF8_STRING"), false)
    , wl_selection(QByteArrayLiteral("_WL_SELECTION"), false)
    , incr(QByteArrayLiteral("INCR"), false)
    , timestamp(QByteArrayLiteral("TIMESTAMP"), false)
    , multiple(QByteArrayLiteral("MULTIPLE"), false)
    , compound_text(QByteArrayLiteral("COMPOUND_TEXT"), false)
    , text(QByteArrayLiteral("TEXT"), false)
    , string(QByteArrayLiteral("STRING"), false)
    , text_plain_utf8(QByteArrayLiteral("text/plain;charset=utf-8"), false)
    , text_plain(QByteArrayLiteral("text/plain"), false)
    , xdnd_selection(QByteArrayLiteral("XdndSelection"), false)
    , xdnd_aware(QByteArrayLiteral("XdndAware"), false)
    , xdnd_enter(QByteArrayLiteral("XdndEnter"), false)
    , xdnd_leave(QByteArrayLiteral("XdndLeave"), false)
    , xdnd_drop(QByteArrayLiteral("XdndDrop"), false)
    , xdnd_status(QByteArrayLiteral("XdndStatus"), false)
    , xdnd_finished(QByteArrayLiteral("XdndFinished"), false)
    , xdnd_type_list(QByteArrayLiteral("XdndTypeList"), false)
    , xdnd_action_copy(QByteArrayLiteral("XdndActionCopy"), false)
    , wl_surface_id(QByteArrayLiteral("WL_SURFACE_ID"), false)
{
}

} // namespace Xcb
