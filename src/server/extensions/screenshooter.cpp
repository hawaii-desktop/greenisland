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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>

#include "core/quickoutput.h"
#include "screenshooter.h"
#include "screenshooter_p.h"

Q_LOGGING_CATEGORY(SCREENSHOOTER_PROTOCOL, "greenisland.protocols.greenisland.screenshooter")

namespace GreenIsland {

namespace Server {

ScreenshooterPrivate::ScreenshooterPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_screenshooter()
{
}

void ScreenshooterPrivate::screenshooter_shoot(Resource *resource, int32_t what,
                                               wl_resource *outputResource,
                                               wl_resource *bufferResource)
{
    Q_Q(Screenshooter);

    Q_UNUSED(what);

    QWaylandOutput *output = QWaylandOutput::fromResource(outputResource);
    QuickOutput *quickOutput = qobject_cast<QuickOutput *>(output);
    if (!quickOutput) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Couldn't find output from resource";
        send_done(resource->handle, result_bad_output);
        Q_EMIT q->done();
        return;
    }

    QQuickWindow *window = static_cast<QQuickWindow *>(quickOutput->window());
    if (!window) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Output doesn't have a QtQuick window thus it cannot be captured";
        send_done(resource->handle, result_bad_output);
        Q_EMIT q->done();
        return;
    }

    wl_shm_buffer *buffer = wl_shm_buffer_get(bufferResource);
    if (!buffer) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Invalid buffer from resource";
        send_done(resource->handle, result_bad_buffer);
        Q_EMIT q->done();
        return;
    }

    uchar *pixels = static_cast<uchar *>(wl_shm_buffer_get_data(buffer));
    int width = wl_shm_buffer_get_width(buffer);
    int height = wl_shm_buffer_get_height(buffer);
    int stride = wl_shm_buffer_get_stride(buffer);
    int bpp = 4;

    if (width < window->width() || height < window->height() || stride < window->width() * bpp) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Buffer is incompatible with output window";
        send_done(resource->handle, result_bad_buffer);
        Q_EMIT q->done();
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    send_done(resource->handle, result_success);
    Q_EMIT q->done();
}

Screenshooter::Screenshooter()
    : QWaylandExtensionTemplate<Screenshooter>(*new ScreenshooterPrivate())
{
}

Screenshooter::Screenshooter(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<Screenshooter>(compositor, *new ScreenshooterPrivate())
{
}

void Screenshooter::initialize()
{
    Q_D(Screenshooter);

    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(SCREENSHOOTER_PROTOCOL) << "Failed to find QWaylandCompositor when initializing Screenshooter";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *Screenshooter::interface()
{
    return ScreenshooterPrivate::interface();
}

QByteArray Screenshooter::interfaceName()
{
    return ScreenshooterPrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland
