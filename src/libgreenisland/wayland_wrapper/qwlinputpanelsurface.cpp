/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwlinputpanelsurface_p.h"

#include "qwloutput_p.h"
#include "qwlsurface_p.h"


namespace GreenIsland {

WlInputPanelSurface::WlInputPanelSurface(wl_client *client, int id, WlSurface *surface)
    : QtWaylandServer::wl_input_panel_surface(client, id, 1)
    , m_surface(surface)
    , m_type(Invalid)
    , m_output(0)
    , m_position()
{
    surface->setInputPanelSurface(this);
}

WlInputPanelSurface::Type WlInputPanelSurface::type() const
{
    return m_type;
}

WlOutput *WlInputPanelSurface::output() const
{
    return m_output;
}

QtWaylandServer::wl_input_panel_surface::position WlInputPanelSurface::position() const
{
    return m_position;
}

void WlInputPanelSurface::input_panel_surface_set_overlay_panel(Resource *)
{
    m_type = OverlayPanel;
}

void WlInputPanelSurface::input_panel_surface_set_toplevel(Resource *, wl_resource *output_resource, uint32_t position)
{
    m_type = Toplevel;
    WlOutputResource *output = static_cast<WlOutputResource *>(WlOutput::Resource::fromResource(output_resource));
    m_output = static_cast<WlOutput *>(output->output_object);
    m_position = static_cast<wl_input_panel_surface::position>(position);
}


} // namespace QtWayland
