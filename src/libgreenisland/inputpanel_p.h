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

#ifndef GREENISLAND_INPUTPANEL_P_H
#define GREENISLAND_INPUTPANEL_P_H

#include <QtCore/private/qobject_p.h>

//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include "wayland_wrapper/qwlsurface_p.h"

#include "qwayland-server-input-method.h"

namespace GreenIsland {

class InputPanel;

class InputPanelPrivate : public QObjectPrivate, public QtWaylandServer::wl_input_panel
{
    Q_DECLARE_PUBLIC(InputPanel)
public:
    InputPanelPrivate();

    GreenIsland::WlSurface *focus() const;
    void setFocus(GreenIsland::WlSurface *focus);

    bool inputPanelVisible() const;
    void setInputPanelVisible(bool inputPanelVisible);

    QRect cursorRectangle() const;
    void setCursorRectangle(const QRect &cursorRectangle);

protected:
    void input_panel_get_input_panel_surface(Resource *resource, uint32_t id, wl_resource *surface) Q_DECL_OVERRIDE;

private:
    GreenIsland::WlSurface *m_focus;
    bool m_inputPanelVisible;
    QRect m_cursorRectangle;
};

}

#endif // GREENISLAND_INPUTPANEL_P_H

