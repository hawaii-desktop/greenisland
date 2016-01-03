/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickItemGrabResult>
#include <QtQuick/QQuickWindow>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandView>

#include "screenshooter.h"
#include "screenshooter_p.h"
#include "serverlogging_p.h"

#define SCREENSHOT_FORMAT WL_SHM_FORMAT_ARGB8888

namespace GreenIsland {

namespace Server {

/*
 * ScreenshooterPrivate
 */

ScreenshooterPrivate::ScreenshooterPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_screenshooter()
{
}

void ScreenshooterPrivate::screenshooter_capture_output(Resource *resource,
                                                        uint32_t id,
                                                        struct ::wl_resource *outputResource,
                                                        wl_array *e)
{
    Q_Q(Screenshooter);

    Screenshooter::Effects effects = 0;

    void *pos;
    wl_array_for_each(pos, e) {
        effects &= static_cast<Screenshooter::Effect>(*reinterpret_cast<quint32 *>(pos));
    }

    Screenshot *screenshot = new Screenshot(Screenshot::CaptureOutput, effects);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    dScreenshot->output = QWaylandOutput::fromResource(outputResource);
    screenshot->setExtensionContainer(q);
    screenshot->initialize();
    dScreenshot->init(resource->client(), id, 1);
    Q_EMIT q->captureRequested(screenshot);
}

void ScreenshooterPrivate::screenshooter_capture_active(Resource *resource,
                                                        uint32_t id,
                                                        wl_array *e)
{
    Q_Q(Screenshooter);

    Screenshooter::Effects effects = 0;

    void *pos;
    wl_array_for_each(pos, e) {
        effects &= static_cast<Screenshooter::Effect>(*reinterpret_cast<quint32 *>(pos));
    }

    Screenshot *screenshot = new Screenshot(Screenshot::CaptureActiveWindow, effects);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    screenshot->setExtensionContainer(q);
    screenshot->initialize();
    dScreenshot->init(resource->client(), id, 1);
    Q_EMIT q->captureRequested(screenshot);
}

void ScreenshooterPrivate::screenshooter_capture_surface(Resource *resource,
                                                         uint32_t id,
                                                         wl_array *e)
{
    Q_Q(Screenshooter);

    Screenshooter::Effects effects = 0;

    void *pos;
    wl_array_for_each(pos, e) {
        effects &= static_cast<Screenshooter::Effect>(*reinterpret_cast<quint32 *>(pos));
    }

    Screenshot *screenshot = new Screenshot(Screenshot::CaptureWindow, effects);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    screenshot->setExtensionContainer(q);
    screenshot->initialize();
    dScreenshot->init(resource->client(), id, 1);
    Q_EMIT q->captureRequested(screenshot);
}

void ScreenshooterPrivate::screenshooter_capture_area(Resource *resource,
                                                      uint32_t id,
                                                      wl_array *e)
{
    Q_Q(Screenshooter);

    Screenshooter::Effects effects = 0;

    void *pos;
    wl_array_for_each(pos, e) {
        effects &= static_cast<Screenshooter::Effect>(*reinterpret_cast<quint32 *>(pos));
    }

    Screenshot *screenshot = new Screenshot(Screenshot::CaptureArea, effects);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    screenshot->setExtensionContainer(q);
    screenshot->initialize();
    dScreenshot->init(resource->client(), id, 1);
    Q_EMIT q->captureRequested(screenshot);
}

/*
 * ScreenshotPrivate
 */

ScreenshotPrivate::ScreenshotPrivate()
    : QWaylandExtensionTemplatePrivate()
    , QtWaylandServer::greenisland_screenshot()
    , output(Q_NULLPTR)
    , selectedSurface(Q_NULLPTR)
    , selectedArea(Q_NULLPTR)
{
}

QImage ScreenshotPrivate::grabItem(QQuickItem *item)
{
    QSharedPointer<QQuickItemGrabResult> result = item->grabToImage();
    if (result.isNull() || result->image().isNull()) {
        QEventLoop loop;
        QObject::connect(result.data(), &QQuickItemGrabResult::ready,
                         &loop, &QEventLoop::quit);
        loop.exec();
    }
    return result->image();
}

void ScreenshotPrivate::screenshot_destroy(Resource *resource)
{
    Q_UNUSED(resource);

    Q_Q(Screenshot);
    q->deleteLater();
}

void ScreenshotPrivate::screenshot_record(Resource *resource,
                                          struct ::wl_resource *bufferResource)
{
    Q_Q(Screenshot);

    wl_shm_buffer *buffer = wl_shm_buffer_get(bufferResource);
    uchar *data = static_cast<uchar *>(wl_shm_buffer_get_data(buffer));
    if (!buffer || !bufferResource || !data) {
        qCWarning(gLcScreenshooter, "Client is trying to record into a null buffer");
        send_failed(error_bad_buffer);
        Q_EMIT q->failed(Screenshot::ErrorBadBuffer);
        return;
    }

    int32_t width = wl_shm_buffer_get_width(buffer);
    int32_t height = wl_shm_buffer_get_height(buffer);
    int32_t stride = wl_shm_buffer_get_stride(buffer);
    uint32_t format = wl_shm_buffer_get_format(buffer);

    // Verify the buffer
    if (stride != width * 4) {
        qCWarning(gLcScreenshooter, "Buffer stride %d doesn't match calculated stride %d",
                  stride, width * 4);
        send_failed(error_bad_buffer);
        Q_EMIT q->failed(Screenshot::ErrorBadBuffer);
        return;
    }
    if (format != SCREENSHOT_FORMAT) {
        qCWarning(gLcScreenshooter, "Wrong buffer format %d, expected %d",
                  format, SCREENSHOT_FORMAT);
        send_failed(error_bad_buffer);
        Q_EMIT q->failed(Screenshot::ErrorBadBuffer);
        return;
    }

    QImage image;

    switch (captureType) {
    case Screenshot::CaptureOutput:
        // Verify the buffer size
        if (width < output->geometry().width() || height < output->geometry().height()) {
            qCWarning(gLcScreenshooter, "Buffer size %dx%d doesn't match output size %dx%d",
                      width, height, output->geometry().width(), output->geometry().height());
            send_failed(error_bad_buffer);
            Q_EMIT q->failed(Screenshot::ErrorBadBuffer);
            return;
        }

        // Capture
        image = static_cast<QQuickWindow *>(output->window())->grabWindow();
        break;
    case Screenshot::CaptureActiveWindow:
    case Screenshot::CaptureWindow:
        image = grabItem(qobject_cast<QQuickItem *>(selectedSurface->views().first()));
        break;
    case Screenshot::CaptureArea:
        image = grabItem(selectedArea);
        break;
    }

    // Handle capture errors
    if (image.isNull()) {
        qCWarning(gLcScreenshooter, "Failed to capture");
        send_failed(error_bad_buffer);
        Q_EMIT q->failed(Screenshot::ErrorBadBuffer);
        return;
    }

    // Dump the image into the buffer
    memcpy(data, image.bits(), height * stride);

    // Done
    send_done(resource->handle);
    Q_EMIT q->done();
}

/*
 * Screenshooter
 */

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
        qCWarning(gLcScreenshooter) << "Failed to find QWaylandCompositor when initializing Screenshooter";
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

/*
 * Screenshot
 */

Screenshot::Screenshot(CaptureType type, Screenshooter::Effects effects)
    : QWaylandExtensionTemplate<Screenshot>(*new ScreenshotPrivate())
{
    d_func()->captureType = type;
    d_func()->effects = effects;
}

Screenshot::CaptureType Screenshot::captureType() const
{
    Q_D(const Screenshot);
    return d->captureType;
}

Screenshooter::Effects Screenshot::effects() const
{
    Q_D(const Screenshot);
    return d->effects;
}

void Screenshot::selectSurface(QWaylandSurface *surface)
{
    Q_D(Screenshot);

    if (d->captureType != CaptureActiveWindow || d->captureType != CaptureWindow) {
        qCWarning(gLcScreenshooter) << "Selecting a surface for a screenshot of an output or an area";
        return;
    }

    d->selectedSurface = surface;
}

void Screenshot::selectArea(QQuickItem *area)
{
    Q_D(Screenshot);

    if (d->captureType != CaptureArea) {
        qCWarning(gLcScreenshooter) << "Selecting an area for a screenshot of an output or a window";
        return;
    }

    d->selectedArea = area;
}

void Screenshot::setup()
{
    Q_D(Screenshot);

    uint32_t width, height;

    // When the client binds we determine the image size based on the capture type
    switch (d->captureType) {
    case CaptureOutput:
        width = d->output->geometry().size().width();
        height = d->output->geometry().size().height();
        break;
    case CaptureActiveWindow:
    case CaptureWindow:
        if (!d->selectedSurface) {
            qCWarning(gLcScreenshooter) << "Please select a surface before calling Screenshot::setup()";
            return;
        }
        width = d->selectedSurface->size().width();
        height = d->selectedSurface->size().height();
        break;
    case CaptureArea:
        if (!d->selectedSurface) {
            qCWarning(gLcScreenshooter) << "Please select an area before calling Screenshot::setup()";
            return;
        }
        width = d->selectedArea->width();
        height = d->selectedArea->height();
        break;
    }

    // Send sizes to the client so it can allocate the buffer
    d->send_setup(width, height, width * 4, SCREENSHOT_FORMAT);
}

const struct wl_interface *Screenshot::interface()
{
    return ScreenshotPrivate::interface();
}

QByteArray Screenshot::interfaceName()
{
    return ScreenshotPrivate::interfaceName();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_screenshooter.cpp"
