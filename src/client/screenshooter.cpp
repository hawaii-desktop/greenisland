/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#include "buffer_p.h"
#include "output.h"
#include "output_p.h"
#include "screenshooter.h"
#include "screenshooter_p.h"

namespace GreenIsland {

namespace Client {

/*
 * ScreenshooterPrivate
 */

ScreenshooterPrivate::ScreenshooterPrivate()
    : QtWayland::greenisland_screenshooter()
    , registry(Q_NULLPTR)
    , shm(Q_NULLPTR)
    , shmPool(Q_NULLPTR)
{
}

ScreenshooterPrivate::~ScreenshooterPrivate()
{
    qDeleteAll(requests);
    delete shmPool;
}

void ScreenshooterPrivate::removeRequest(Screenshot *screenshot)
{
    QMutexLocker locker(&requestsMutex);
    requests.removeOne(screenshot);
}

/*
 * Screenshooter
 */

Screenshooter::Screenshooter(Shm *shm, QObject *parent)
    : QObject(*new ScreenshooterPrivate(), parent)
{
    d_func()->shm = shm;
    d_func()->shmPool = shm->createPool(1024);
}

Screenshot *Screenshooter::captureOutput(Output *output)
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureOutput, this);
    ScreenshotPrivate::get(screenshot)->output = output;
    ScreenshotPrivate::get(screenshot)->init(d->capture_output(OutputPrivate::get(output)->object()));
    d->requests.append(screenshot);
    return screenshot;
}

Screenshot *Screenshooter::captureActiveWindow()
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureActiveWindow, this);
    ScreenshotPrivate::get(screenshot)->init(d->capture_active());
    d->requests.append(screenshot);
    return screenshot;
}

Screenshot *Screenshooter::captureWindow()
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureWindow, this);
    ScreenshotPrivate::get(screenshot)->init(d->capture_surface());
    d->requests.append(screenshot);
    return screenshot;
}

Screenshot *Screenshooter::captureArea()
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureArea, this);
    ScreenshotPrivate::get(screenshot)->init(d->capture_area());
    d->requests.append(screenshot);
    return screenshot;
}

QByteArray Screenshooter::interfaceName()
{
    return QByteArrayLiteral("greenisland_screenshooter");
}

/*
 * ScreenshotPrivate
 */

ScreenshotPrivate::ScreenshotPrivate(Screenshooter *s)
    : QtWayland::greenisland_screenshot()
    , screenshooter(s)
    , output(Q_NULLPTR)
{
}

ScreenshotPrivate::~ScreenshotPrivate()
{
    Q_Q(Screenshot);
    ScreenshooterPrivate::get(screenshooter)->removeRequest(q);
    destroy();
}

void ScreenshotPrivate::screenshot_setup(int32_t width, int32_t height, int32_t stride, int32_t f)
{
    Q_Q(Screenshot);

    Shm::Format format = static_cast<Shm::Format>(f);
    ScreenshooterPrivate *dScreenshooter = ScreenshooterPrivate::get(screenshooter);

    // The compositor should give us a supported format (in theory)
    // but who knows, better check it
    if (!Q_UNLIKELY(dScreenshooter->shm->formats().contains(format))) {
        qWarning("Unsupported format %s", qPrintable(Shm::formatToString(format)));
        Q_EMIT q->setupFailed();
        return;
    }

    // Create the buffer
    buffer = dScreenshooter->shmPool->createBuffer(
                QSize(width, height),
                stride, Q_NULLPTR, format);
    if (buffer.isNull()) {
        qWarning("Failed to create %dx%d buffer (stride %d) with format %s",
                 width, height, stride, qPrintable(Shm::formatToString(format)));
        Q_EMIT q->setupFailed();
        return;
    }

    // Ask the compositor to capture
    record(BufferPrivate::get(buffer.data())->object());

    Q_EMIT q->setupDone(QSize(width, height), stride);
}

void ScreenshotPrivate::screenshot_done()
{
    Q_Q(Screenshot);
    Q_EMIT q->done(buffer.data());
}

void ScreenshotPrivate::screenshot_failed(int32_t error)
{
    Q_Q(Screenshot);
    Q_EMIT q->failed(static_cast<Screenshot::Error>(error));
}

/*
 * Screenshot
 */

Screenshot::Screenshot(CaptureType type, QObject *parent)
    : QObject(*new ScreenshotPrivate(static_cast<Screenshooter *>(parent)), parent)
{
    d_func()->type = type;
}

Screenshot::CaptureType Screenshot::captureType() const
{
    Q_D(const Screenshot);
    return d->type;
}

Output *Screenshot::output() const
{
    Q_D(const Screenshot);
    return d->output;
}

QByteArray Screenshot::interfaceName()
{
    return QByteArrayLiteral("greenisland_screenshot");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_screenshooter.cpp"
