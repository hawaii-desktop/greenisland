/****************************************************************************
 * This file is part of Green Island.
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#include "compositor.h"
#include "inputpanel.h"
#include "inputpanel_p.h"
#include "wayland_wrapper/qwlinputpanelsurface_p.h"

namespace GreenIsland {

/*
 * InputPanelPrivate
 */

InputPanelPrivate::InputPanelPrivate()
    : QtWaylandServer::wl_input_panel(Compositor::instance()->waylandDisplay(), 1)
    , m_focus()
    , m_inputPanelVisible(false)
    , m_cursorRectangle()
{
}

WlSurface *InputPanelPrivate::focus() const
{
    return m_focus;
}

void InputPanelPrivate::setFocus(WlSurface *focus)
{
    Q_Q(InputPanel);

    if (m_focus == focus)
        return;

    m_focus = focus;

    Q_EMIT q->focusChanged();
}

bool InputPanelPrivate::inputPanelVisible() const
{
    return m_inputPanelVisible;
}

void InputPanelPrivate::setInputPanelVisible(bool inputPanelVisible)
{
    Q_Q(InputPanel);

    if (m_inputPanelVisible == inputPanelVisible)
        return;

    m_inputPanelVisible = inputPanelVisible;

    Q_EMIT q->visibleChanged();
}

QRect InputPanelPrivate::cursorRectangle() const
{
    return m_cursorRectangle;
}

void InputPanelPrivate::setCursorRectangle(const QRect &cursorRectangle)
{
    Q_Q(InputPanel);

    if (m_cursorRectangle == cursorRectangle)
        return;

    m_cursorRectangle = cursorRectangle;

    Q_EMIT q->cursorRectangleChanged();
}

void InputPanelPrivate::input_panel_get_input_panel_surface(Resource *resource, uint32_t id, wl_resource *surface)
{
    new WlInputPanelSurface(resource->client(), id,
                            WlSurface::fromResource(surface));
}

/*
 * InputPanel
 */

InputPanel::InputPanel(QObject *parent)
    : QObject(*new InputPanelPrivate, parent)
{
}

Surface *InputPanel::focus() const
{
    Q_D(const InputPanel);

    WlSurface *surface = d->focus();
    if (surface)
        return surface->waylandSurface();

    return Q_NULLPTR;
}

bool InputPanel::visible() const
{
    Q_D(const InputPanel);

    return d->inputPanelVisible();
}

QRect InputPanel::cursorRectangle() const
{
    Q_D(const InputPanel);

    return d->cursorRectangle();
}

}

#include "moc_inputpanel.cpp"
