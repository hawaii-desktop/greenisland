/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "output.h"
#include "output_p.h"

namespace GreenIsland {

namespace Client {

static Output::Subpixel toSubpixel(const QtWayland::wl_output::subpixel &value)
{
    switch (value) {
    case QtWayland::wl_output::subpixel_unknown:
        return Output::SubpixelUnknown;
    case QtWayland::wl_output::subpixel_none:
        return Output::SubpixelNone;
    case QtWayland::wl_output::subpixel_horizontal_rgb:
        return Output::SubpixelHorizontalRgb;
    case QtWayland::wl_output::subpixel_horizontal_bgr:
        return Output::SubpixelHorizontalBgr;
    case QtWayland::wl_output::subpixel_vertical_rgb:
        return Output::SubpixelVerticalRgb;
    case QtWayland::wl_output::subpixel_vertical_bgr:
        return Output::SubpixelVerticalBgr;
    default:
        break;
    }

    return Output::SubpixelUnknown;
}

static Output::Transform toTransform(const QtWayland::wl_output::transform &value)
{
    switch (value) {
    case QtWayland::wl_output::transform_90:
        return Output::Transform90;
    case QtWayland::wl_output::transform_180:
        return Output::Transform180;
    case QtWayland::wl_output::transform_270:
        return Output::Transform270;
    case QtWayland::wl_output::transform_flipped:
        return Output::TransformFlipped;
    case QtWayland::wl_output::transform_flipped_90:
        return Output::TransformFlipped90;
    case QtWayland::wl_output::transform_flipped_180:
        return Output::TransformFlipped180;
    case QtWayland::wl_output::transform_flipped_270:
        return Output::TransformFlipped270;
    default:
        break;
    }

    return Output::TransformNormal;
}

/*
 * OutputPrivate
 */

OutputPrivate::OutputPrivate()
    : QtWayland::wl_output()
    , scale(1)
    , currentMode(modes.end())
    , subpixelValue(Output::SubpixelUnknown)
    , transformValue(Output::TransformNormal)
{
}

Output *OutputPrivate::fromWlOutput(struct ::wl_output *output)
{
    QtWayland::wl_output *wlOutput =
            static_cast<QtWayland::wl_output *>(wl_output_get_user_data(output));
    return static_cast<OutputPrivate *>(wlOutput)->q_func();
}

void OutputPrivate::output_geometry(int32_t x, int32_t y,
                                    int32_t physical_width, int32_t physical_height,
                                    int32_t _subpixel, const QString &make,
                                    const QString &_model, int32_t _transform)
{
    Q_Q(Output);

    if (manufacturer != make) {
        manufacturer = make;
        Q_EMIT q->manufacturerChanged();
    }

    if (model != _model) {
        model = _model;
        Q_EMIT q->modelChanged();
    }

    if (position.x() != x || position.y() != y) {
        position = QPoint(x, y);
        Q_EMIT q->positionChanged();
    }

    if (physicalSize.width() != physical_width || physicalSize.height() != physical_height) {
        physicalSize = QSize(physical_width, physical_height);
        Q_EMIT q->physicalSizeChanged();
    }

    auto newSubpixel = toSubpixel(static_cast<QtWayland::wl_output::subpixel>(_subpixel));
    if (subpixelValue != newSubpixel) {
        subpixelValue = newSubpixel;
        Q_EMIT q->subpixelChanged();
    }

    auto newTransform = toTransform(static_cast<QtWayland::wl_output::transform>(_transform));
    if (transformValue != newTransform) {
        transformValue = newTransform;
        Q_EMIT q->transformChanged();
    }
}

void OutputPrivate::output_mode(uint32_t flags, int32_t width, int32_t height,
                                int32_t refresh)
{
    Q_Q(Output);

    Output::Mode mode;
    mode.flags = Output::ModeFlags(flags);
    mode.size = QSize(width, height);
    mode.refreshRate = qreal(refresh) / 1000;

    bool found = false;
    bool changed = false;

    for (auto it = modes.begin(); it != modes.end(); it++) {
        Output::Mode &m = (*it);

        if (m.size == mode.size && m.refreshRate == mode.refreshRate) {
            found = true;

            // Update flags if the mode is already present
            if (m.flags != mode.flags) {
                m.flags = mode.flags;
                changed = true;
            }
        }
    }

    // Append the mode
    if (!found) {
        auto it = modes.insert(modes.end(), mode);
        if (mode.flags.testFlag(Output::CurrentMode))
            currentMode = it;
    }

    if (changed)
        Q_EMIT q->modeChanged(mode);
    else if (!found)
        Q_EMIT q->modeAdded(mode);

    if (changed || (!found || mode.flags.testFlag(Output::CurrentMode))) {
        Q_EMIT q->sizeChanged();
        Q_EMIT q->geometryChanged();
        Q_EMIT q->refreshRateChanged();
    }
}

void OutputPrivate::output_done()
{
    Q_Q(Output);

    // The done event is sent by the server in order to make
    // properties changes feel like they are atomic
    Q_EMIT q->outputChanged();
}

void OutputPrivate::output_scale(int32_t factor)
{
    Q_Q(Output);

    if (scale != factor) {
        scale = factor;
        Q_EMIT q->scaleChanged();
    }
}

/*
 * Output
 */

Output::Output(QObject *parent)
    : QObject(*new OutputPrivate(), parent)
{
}

QString Output::manufacturer() const
{
    Q_D(const Output);
    return d->manufacturer;
}

QString Output::model() const
{
    Q_D(const Output);
    return d->model;
}

QPoint Output::position() const
{
    Q_D(const Output);
    return d->position;
}

QSize Output::size() const
{
    Q_D(const Output);
    if (d->currentMode == d->modes.end())
        return QSize();
    return d->currentMode->size;
}

QRect Output::geometry() const
{
    Q_D(const Output);
    if (d->currentMode == d->modes.end())
        return QRect();
    return QRect(d->position, d->currentMode->size);
}

qreal Output::refreshRate() const
{
    Q_D(const Output);
    if (d->currentMode == d->modes.end())
        return 0;
    return d->currentMode->refreshRate;
}

QSize Output::physicalSize() const
{
    Q_D(const Output);
    return d->physicalSize;
}

qint32 Output::scale() const
{
    Q_D(const Output);
    return d->scale;
}

QList<Output::Mode> Output::modes() const
{
    Q_D(const Output);
    return d->modes.toList();
}

Output::Subpixel Output::subpixel() const
{
    Q_D(const Output);
    return d->subpixelValue;
}

Output::Transform Output::transform() const
{
    Q_D(const Output);
    return d->transformValue;
}

Output *Output::fromQt(QScreen *screen, QObject *parent)
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland")))
        return Q_NULLPTR;

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (!native)
        return Q_NULLPTR;

    wl_output *output = reinterpret_cast<wl_output *>(
                native->nativeResourceForScreen(QByteArrayLiteral("output"), screen));
    if (!output)
        return Q_NULLPTR;

    Output *o = new Output(parent);
    OutputPrivate::get(o)->init(output);
    return o;
}

QByteArray Output::interfaceName()
{
    return QByteArrayLiteral("wl_output");
}

} // namespace Client

} // namespace GreenIsland

#include "moc_output.cpp"
