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
 ***************************************************************************/

#include <QtCompositor/QWaylandQuickOutput>

#include "greenislandscreenshooter.h"

Q_LOGGING_CATEGORY(SCREENSHOOTER_PROTOCOL, "greenisland.protocols.greenisland.screenshooter")

namespace GreenIsland {

GreenIslandScreenshooterGlobal::GreenIslandScreenshooterGlobal(QObject *parent)
    : QObject(parent)
{
}

const wl_interface *GreenIslandScreenshooterGlobal::interface() const
{
    return &greenisland_screenshooter_interface;
}

void GreenIslandScreenshooterGlobal::bind(wl_client *client, uint32_t version, uint32_t id)
{
    new GreenIslandScreenshooter(client, id, version, this);
}

GreenIslandScreenshooter::GreenIslandScreenshooter(wl_client *client, uint32_t name, uint32_t version, QObject *parent)
    : QObject(parent)
    , QtWaylandServer::greenisland_screenshooter(client, name, version)
{
}

GreenIslandScreenshooter::~GreenIslandScreenshooter()
{
}

void GreenIslandScreenshooter::screenshooter_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void GreenIslandScreenshooter::screenshooter_shoot(Resource *resource,
                                                   wl_resource *outputResource,
                                                   wl_resource *bufferResource)
{
    Q_UNUSED(resource)

    QWaylandQuickOutput *output = static_cast<QWaylandQuickOutput *>(QWaylandOutput::fromResource(outputResource));
    if (!output) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Couldn't find output from resource";
        send_done(result_bad_output);
        return;
    }

    QQuickWindow *window = output->quickWindow();
    if (!window) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Output doesn't have a window thus it cannot be captured";
        send_done(result_bad_output);
        return;
    }

    wl_shm_buffer *buffer = wl_shm_buffer_get(bufferResource);
    if (!buffer) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Invalid buffer from resource";
        send_done(result_bad_buffer);
        return;
    }

    uchar *pixels = static_cast<uchar *>(wl_shm_buffer_get_data(buffer));
    int width = wl_shm_buffer_get_width(buffer);
    int height = wl_shm_buffer_get_height(buffer);
    int stride = wl_shm_buffer_get_stride(buffer);
    int bpp = 4;

    if (width < window->width() || height < window->height() || stride < window->width() * bpp) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Buffer is incompatible with output window";
        send_done(result_bad_buffer);
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    send_done(result_success);
}

}
