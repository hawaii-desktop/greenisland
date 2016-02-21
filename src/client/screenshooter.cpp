/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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

QByteArray ScreenshooterPrivate::effectsArray(Screenshooter::Effects effects)
{
    wl_array array;
    quint32 *e;

    wl_array_init(&array);
    if (effects.testFlag(Screenshooter::EffectPointer)) {
        e = (quint32 *)wl_array_add(&array, sizeof(*e));
        *e = ScreenshooterPrivate::effects_pointer;
    }
    if (effects.testFlag(Screenshooter::EffectBorder)) {
        e = (quint32 *)wl_array_add(&array, sizeof(*e));
        *e = ScreenshooterPrivate::effects_border;
    }
    QByteArray data((const char *)array.data, array.size);
    wl_array_release(&array);

    return data;
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

Screenshot *Screenshooter::captureOutput(Output *output, Effects effects)
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureOutput,
                                            effects, this);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    dScreenshot->output = output;
    dScreenshot->init(d->capture_output(OutputPrivate::get(output)->object(), d->effectsArray(effects)));
    d->requests.append(screenshot);
    return screenshot;
}

Screenshot *Screenshooter::captureActiveWindow(Effects effects)
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureActiveWindow,
                                            effects, this);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    dScreenshot->init(d->capture_active(d->effectsArray(effects)));
    d->requests.append(screenshot);
    return screenshot;
}

Screenshot *Screenshooter::captureWindow(Effects effects)
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureWindow,
                                            effects, this);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    dScreenshot->init(d->capture_surface(d->effectsArray(effects)));
    d->requests.append(screenshot);
    return screenshot;
}

Screenshot *Screenshooter::captureArea(Effects effects)
{
    Q_D(Screenshooter);
    Screenshot *screenshot = new Screenshot(Screenshot::CaptureArea,
                                            effects, this);
    ScreenshotPrivate *dScreenshot = ScreenshotPrivate::get(screenshot);
    dScreenshot->init(d->capture_area(d->effectsArray(effects)));
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

Screenshot::Screenshot(CaptureType type, Screenshooter::Effects effects, QObject *parent)
    : QObject(*new ScreenshotPrivate(static_cast<Screenshooter *>(parent)), parent)
{
    d_func()->type = type;
    d_func()->effects = effects;
}

Screenshot::CaptureType Screenshot::captureType() const
{
    Q_D(const Screenshot);
    return d->type;
}

Screenshooter::Effects Screenshot::effects() const
{
    Q_D(const Screenshot);
    return d->effects;
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
