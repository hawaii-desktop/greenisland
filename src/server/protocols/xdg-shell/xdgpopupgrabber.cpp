/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies)
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *     Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
 *
 *     You may use this file under the terms of the BSD license as follows:
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in
 *         the documentation and/or other materials provided with the
 *         distribution.
 *       * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 *         of its contributors may be used to endorse or promote products derived
 *         from this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ***************************************************************************/

#include <QtCompositor/QWaylandSurfaceView>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "xdgpopupgrabber.h"
#include "xdgpopup.h"

namespace GreenIsland {

XdgPopupGrabber::XdgPopupGrabber(QtWayland::InputDevice *inputDevice)
    : QtWayland::PointerGrabber()
    , m_inputDevice(inputDevice)
    , m_client(Q_NULLPTR)
    , m_initialUp(false)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;
}

uint32_t XdgPopupGrabber::serial() const
{
    return m_inputDevice->pointerDevice()->grabSerial();
}

void XdgPopupGrabber::addPopup(XdgPopup *popup)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    if (m_popupSurfaces.contains(popup))
        return;

    if (m_popupSurfaces.isEmpty()) {
        m_client = popup->resource()->client();
        m_initialUp = !m_inputDevice->pointerDevice()->buttonPressed();
        m_popupSurfaces.append(popup);
        m_inputDevice->pointerDevice()->startGrab(this);
    } else {
        m_popupSurfaces.append(popup);
    }
}

void XdgPopupGrabber::removePopup(XdgPopup *popup)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    if (m_popupSurfaces.isEmpty())
        return;

    m_popupSurfaces.removeOne(popup);

    if (m_popupSurfaces.isEmpty())
        m_inputDevice->pointerDevice()->endGrab();
}

void XdgPopupGrabber::focus()
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    if (m_pointer->current() && m_pointer->current()->surface()->handle()->resource()->client() == m_client)
        m_pointer->setFocus(m_pointer->current(), m_pointer->currentPosition());
    else
        m_pointer->setFocus(Q_NULLPTR, QPointF());
}

void XdgPopupGrabber::motion(uint32_t time)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    m_pointer->motion(time);
}

void XdgPopupGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    qCDebug(XDGSHELL_TRACE) << Q_FUNC_INFO;

    if (m_pointer->focusResource()) {
        m_pointer->sendButton(time, button, state);
    } else if (state == QtWaylandServer::wl_pointer::button_state_pressed &&
               (m_initialUp || time - m_pointer->grabTime() > 500) &&
               m_pointer->currentGrab() == this) {
        m_pointer->endGrab();

        for (XdgPopup *popup: m_popupSurfaces)
            popup->done();
        m_popupSurfaces.clear();
    }

    if (state == QtWaylandServer::wl_pointer::button_state_released)
        m_initialUp = false;
}

}