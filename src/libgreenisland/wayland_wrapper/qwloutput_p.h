/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef WL_OUTPUT_H
#define WL_OUTPUT_H

#include <greenisland/greenisland_export.h>

#include <QtCore/QRect>
#include <QtCore/QList>

#include "qwayland-server-wayland.h"
#include "abstractoutput.h"


class QWindow;

namespace GreenIsland {

class WlCompositor;

struct WlOutputResource : public QtWaylandServer::wl_output::Resource
{
    WlOutputResource() {}
};

class WlOutput : public QtWaylandServer::wl_output
{
public:
    explicit WlOutput(WlCompositor *compositor, QWindow *window = 0);

    WlCompositor *compositor() const { return m_compositor; }

    AbstractOutput *output() const { return m_output; }

    QString manufacturer() const { return m_manufacturer; }
    void setManufacturer(const QString &manufacturer);

    QString model() const { return m_model; }
    void setModel(const QString &model);

    QPoint position() const { return m_position; }
    void setPosition(const QPoint &position);

    QRect geometry() const;
    void setGeometry(const QRect &geometry);

    AbstractOutput::Mode mode() const { return m_mode; }
    void setMode(const AbstractOutput::Mode &mode);

    QRect availableGeometry() const { return m_availableGeometry; }
    void setAvailableGeometry(const QRect &availableGeometry);

    QSize physicalSize() const { return m_physicalSize; }
    void setPhysicalSize(const QSize &physicalSize);

    AbstractOutput::Subpixel subpixel() const { return m_subpixel; }
    void setSubpixel(const AbstractOutput::Subpixel &subpixel);

    AbstractOutput::Transform transform() const { return m_transform; }
    void setTransform(const AbstractOutput::Transform &transform);

    int scaleFactor() const { return m_scaleFactor; }
    void setScaleFactor(int scale);

    QWindow *window() const { return m_window; }

    WlOutputResource *outputForClient(struct wl_client *client) const;

    AbstractOutput *waylandOutput() const { return m_output; }

    void output_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
    Resource *output_allocate() Q_DECL_OVERRIDE { return new WlOutputResource; }

private:
    friend class AbstractOutput;

    WlCompositor *m_compositor;
    QWindow *m_window;
    AbstractOutput *m_output;
    QString m_manufacturer;
    QString m_model;
    QPoint m_position;
    AbstractOutput::Mode m_mode;
    QRect m_availableGeometry;
    QSize m_physicalSize;
    AbstractOutput::Subpixel m_subpixel;
    AbstractOutput::Transform m_transform;
    int m_scaleFactor;
    QList<Surface *> m_surfaces;

    void sendGeometryInfo();
};

}


#endif //WL_OUTPUT_H
