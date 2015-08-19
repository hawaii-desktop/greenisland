/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "compositor.h"
#include "output.h"
#include "output_p.h"
#include "outputwindow.h"

#include "wayland_wrapper/qwlcompositor_p.h"

namespace GreenIsland {

/*
 * OutputPrivate
 */

static QtWaylandServer::wl_output::subpixel toWlSubpixel(const Output::Subpixel &value)
{
    switch (value) {
    case Output::SubpixelUnknown:
        return QtWaylandServer::wl_output::subpixel_unknown;
    case Output::SubpixelNone:
        return QtWaylandServer::wl_output::subpixel_none;
    case Output::SubpixelHorizontalRgb:
        return QtWaylandServer::wl_output::subpixel_horizontal_rgb;
    case Output::SubpixelHorizontalBgr:
        return QtWaylandServer::wl_output::subpixel_horizontal_bgr;
    case Output::SubpixelVerticalRgb:
        return QtWaylandServer::wl_output::subpixel_vertical_rgb;
    case Output::SubpixelVerticalBgr:
        return QtWaylandServer::wl_output::subpixel_vertical_bgr;
    default:
        break;
    }

    return QtWaylandServer::wl_output::subpixel_unknown;
}

static QtWaylandServer::wl_output::transform toWlTransform(const Output::Transform &value)
{
    switch (value) {
    case Output::Transform90:
        return QtWaylandServer::wl_output::transform_90;
    case Output::Transform180:
        return QtWaylandServer::wl_output::transform_180;
    case Output::Transform270:
        return QtWaylandServer::wl_output::transform_270;
    case Output::TransformFlipped:
        return QtWaylandServer::wl_output::transform_flipped;
    case Output::TransformFlipped90:
        return QtWaylandServer::wl_output::transform_flipped_90;
    case Output::TransformFlipped180:
        return QtWaylandServer::wl_output::transform_flipped_180;
    case Output::TransformFlipped270:
        return QtWaylandServer::wl_output::transform_flipped_270;
    default:
        break;
    }

    return QtWaylandServer::wl_output::transform_normal;
}

OutputPrivate::OutputPrivate()
    : QtWaylandServer::wl_output(Compositor::instance()->waylandDisplay(), 2)
    , name(QStringLiteral("Output"))
    , manufacturer(QStringLiteral("Hawaii"))
    , model(QStringLiteral("None"))
    , primary(false)
    , position(QPoint(0, 0))
    , subpixel(Output::SubpixelUnknown)
    , transform(Output::TransformNormal)
    , scaleFactor(1)
    , updateScheduled(false)
    , hotSpotSize(QSize(10, 10))
    , hotSpotThreshold(1000)
    , hotSpotPushTime(150)
{
    static int seed = 0;
    id = seed++;
}

void OutputPrivate::sendGeometryInfo(Resource *resource, bool sendDone)
{
    send_geometry(resource->handle,
                  position.x(), position.x(),
                  physicalSize.width(), physicalSize.height(),
                  toWlSubpixel(subpixel), manufacturer, model,
                  toWlTransform(transform));

    if (sendDone && resource->version() >= 2)
        send_done(resource->handle);
}

void OutputPrivate::sendAllGeometryInfo(bool sendDone)
{
    Q_FOREACH (Resource *resource, resourceMap().values())
        sendGeometryInfo(resource, sendDone);
}

void OutputPrivate::sendScaleFactor(Resource *resource, bool sendDone)
{
    if (resource->version() >= 2) {
        send_scale(resource->handle, scaleFactor);

        if (sendDone)
            send_done(resource->handle);
    }
}

void OutputPrivate::sendAllScaleFactor(bool sendDone)
{
    Q_FOREACH (Resource *resource, resourceMap().values())
        sendScaleFactor(resource, sendDone);
}

void OutputPrivate::sendModes(Resource *resource, bool sendDone)
{
    auto currentModeIt = modes.constEnd();
    for (auto it = modes.constBegin(); it != modes.constEnd(); ++it) {
        const Output::Mode &mode = *it;

        // Current mode is last
        if (mode.flags.testFlag(Output::ModeFlag::CurrentMode)) {
            currentModeIt = it;
            continue;
        }

        sendMode(resource, mode);
    }

    // Send the current mode
    if (currentModeIt != modes.constEnd())
        sendMode(resource, *currentModeIt);

    if (sendDone && resource->version() >= 2)
        send_done(resource->handle);
}

void OutputPrivate::sendAllModes(bool sendDone)
{
    Q_FOREACH (Resource *resource, resourceMap().values())
        sendModes(resource, sendDone);
}

void OutputPrivate::sendMode(Resource *resource, const Output::Mode &mode)
{
    quint32 flags = 0;
    if (mode.flags.testFlag(Output::ModeFlag::CurrentMode))
        flags |= quint32(QtWaylandServer::wl_output::mode_current);
    if (mode.flags.testFlag(Output::ModeFlag::PreferredMode))
        flags |= quint32(QtWaylandServer::wl_output::mode_preferred);

    send_mode(resource->handle, flags,
              mode.size.width(), mode.size.height(),
              mode.refreshRate);
}

void OutputPrivate::setPrimary(bool value)
{
    Q_Q(Output);

    if (primary == value)
        return;

    primary = value;
    Q_EMIT q->primaryChanged();
}

void OutputPrivate::update()
{
    if (!updateScheduled) {
        /*
        QRect rect(QPoint(0, 0), window->size());
        QRegion region(rect);
        QExposeEvent *event = new QExposeEvent(region);
        QCoreApplication::postEvent(window, event);
        */

        window->update();

        updateScheduled = true;
    }
}

void OutputPrivate::_q_updateStarted()
{
    updateScheduled = false;
    Compositor::instance()->frameStarted();
    Compositor::instance()->cleanupGraphicsResources();
}

void OutputPrivate::output_bind_resource(Resource *resource)
{
    sendGeometryInfo(resource);
    sendScaleFactor(resource);
    sendModes(resource);

    if (resource->version() >= 2)
        send_done(resource->handle);
}

/*
 * Output
 */

Output::Output(const QString &name,
               const QString &manufacturer, const QString &model)
    : QObject(*new OutputPrivate)
{
    Q_D(Output);

    // Set output information
    if (!name.isEmpty())
        d->name = name;
    if (!manufacturer.isEmpty())
        d->manufacturer = manufacturer;
    if (!model.isEmpty())
        d->model = model;

    // Setup the window
    d->window = new OutputWindow(this);
    d->window->setTitle(QString(QStringLiteral("Green Island - %1")).arg(name));

    // Add this output
    Compositor::instance()->handle()->addOutput(this);

    // Update the window before synchronizing
    connect(d->window, SIGNAL(beforeSynchronizing()),
            this, SLOT(_q_updateStarted()),
            Qt::DirectConnection);

    // Resize the window
    connect(this, &Output::sizeChanged, this, [this, d] {
        QSize newSize = size();
        d->window->resize(newSize);
        d->window->setMinimumSize(newSize);
        d->window->setMaximumSize(newSize);
    });
}

Output::~Output()
{
    Compositor::instance()->handle()->removeOutput(this);
}

QString Output::name() const
{
    Q_D(const Output);
    return d->name;
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

int Output::number() const
{
    Q_D(const Output);
    return d->id;
}

bool Output::isPrimary() const
{
    Q_D(const Output);
    return d->primary;
}

QPoint Output::position() const
{
    Q_D(const Output);
    return d->position;
}

void Output::setPosition(const QPoint &pt)
{
    Q_D(Output);

    if (d->position == pt)
        return;

    d->position = pt;
    d->sendAllGeometryInfo(true);
    Q_EMIT positionChanged();
    Q_EMIT geometryChanged();
}

QSize Output::size() const
{
    Q_D(const Output);

    auto it = std::find_if(d->modes.begin(), d->modes.end(), [](const Mode &mode) {
        return mode.flags.testFlag(ModeFlag::CurrentMode);
    });

    // Return an empty size when we don't have a current mode
    if (it == d->modes.end())
        return QSize();

    return (*it).size;
}

int Output::refreshRate() const
{
    Q_D(const Output);

    auto it = std::find_if(d->modes.begin(), d->modes.end(), [](const Mode &mode) {
        return mode.flags.testFlag(ModeFlag::CurrentMode);
    });

    // Return 60Hz when we don't have a current mode
    if (it == d->modes.end())
        return 60000;

    return (*it).refreshRate;
}

QList<Output::Mode> Output::modes() const
{
    Q_D(const Output);
    return d->modes;
}

void Output::addMode(const QSize &size, ModeFlags flags, int refreshRate)
{
    Q_D(Output);

    auto it = std::find_if(d->modes.begin(), d->modes.end(), [](const Mode &mode) {
        return mode.flags.testFlag(ModeFlag::CurrentMode);
    });

    // Enforce the current flag if no mode has such flag
    if (it == d->modes.end() && !flags.testFlag(ModeFlag::CurrentMode))
        flags |= ModeFlag::CurrentMode;

    // Remove the current flag if another mode already has it
    if (it != d->modes.end() && flags.testFlag(ModeFlag::CurrentMode))
        (*it).flags &=  ~uint(ModeFlag::CurrentMode);

    // Remove from an existing preferred mode
    if (flags.testFlag(ModeFlag::PreferredMode)) {
        auto preferredIt = std::find_if(d->modes.begin(), d->modes.end(), [](const Mode &mode) {
            return mode.flags.testFlag(ModeFlag::PreferredMode);
        });

        if (preferredIt != d->modes.end())
            (*preferredIt).flags &= ~uint(ModeFlag::PreferredMode);
    }

    // Function that emits the signals
    auto emitModeChangedFunc = [this, flags] {
        Q_EMIT modesChanged();
        if (flags.testFlag(ModeFlag::CurrentMode)) {
            // Signal changes
            Q_EMIT sizeChanged();
            Q_EMIT refreshRateChanged();
            Q_EMIT currentModeChanged();
            Q_EMIT geometryChanged();
        }
    };

    // Try to find an existing mode
    auto foundIt = std::find_if(d->modes.begin(), d->modes.end(), [size, refreshRate](const Mode &mode) {
        return mode.size == size && mode.refreshRate == refreshRate;
    });

    // Change flags to an existing mode
    if (foundIt != d->modes.end()) {
        // Just return if flags didn't change
        if ((*foundIt).flags == flags)
            return;

        (*foundIt).flags = flags;
        emitModeChangedFunc();
        return;
    }

    // Append the new mode
    Mode mode;
    mode.size = size;
    mode.refreshRate = refreshRate;
    mode.flags = flags;
    d->modes.append(mode);
    emitModeChangedFunc();
}

void Output::setCurrentMode(const QSize &size, int refreshRate)
{
    Q_D(Output);

    auto currentIt = std::find_if(d->modes.begin(), d->modes.end(), [](const Mode &mode) {
        return mode.flags.testFlag(ModeFlag::CurrentMode);
    });

    // Find a mode with that size and refresh rate and set it as current
    auto existingIt = std::find_if(d->modes.begin(), d->modes.end(), [size, refreshRate](const Mode &mode) {
        return mode.size == size && mode.refreshRate == refreshRate;
    });
    if (existingIt == d->modes.end())
        return;
    (*existingIt).flags |= ModeFlag::CurrentMode;

    // Previous current mode found, remove the flag
    if (currentIt != d->modes.end())
        (*currentIt).flags &= ~uint(ModeFlag::CurrentMode);

    // Signal changes
    Q_EMIT modesChanged();
    Q_EMIT sizeChanged();
    Q_EMIT refreshRateChanged();
    Q_EMIT currentModeChanged();
    Q_EMIT geometryChanged();
}

QRect Output::geometry() const
{
    Q_D(const Output);
    return QRect(d->position, size());
}

QRect Output::availableGeometry() const
{
    Q_D(const Output);

    if (!d->availableGeometry.isValid())
        return QRect(d->position, size());

    return d->availableGeometry;
}

void Output::setAvailableGeometry(const QRect &availableGeometry)
{
    Q_D(Output);

    if (d->availableGeometry == availableGeometry)
        return;

    d->availableGeometry = availableGeometry;
    Q_EMIT availableGeometryChanged();
}

QSize Output::physicalSize() const
{
    Q_D(const Output);
    return d->physicalSize;
}

void Output::setPhysicalSize(const QSize &size)
{
    Q_D(Output);

    if (d->physicalSize == size)
        return;

    d->physicalSize = size;
    d->sendAllGeometryInfo(true);
    Q_EMIT physicalSizeChanged();
}

Output::Subpixel Output::subpixel() const
{
    Q_D(const Output);
    return d->subpixel;
}

void Output::setSubpixel(const Subpixel &subpixel)
{
    Q_D(Output);

    if (d->subpixel == subpixel)
        return;

    d->subpixel = subpixel;
    d->sendAllGeometryInfo(true);
    Q_EMIT subpixelChanged();
}

Output::Transform Output::transform() const
{
    Q_D(const Output);
    return d->transform;
}

void Output::setTransform(const Transform &transform)
{
    Q_D(Output);

    if (d->transform == transform)
        return;

    d->transform = transform;
    d->sendAllGeometryInfo(true);
    Q_EMIT transformChanged();
}

int Output::scaleFactor() const
{
    Q_D(const Output);
    return d->scaleFactor;
}

void Output::setScaleFactor(int scale)
{
    Q_D(Output);

    if (d->scaleFactor == scale)
        return;

    d->scaleFactor = scale;
    d->sendAllScaleFactor(true);
    Q_EMIT scaleFactorChanged();
}

QSize Output::hotSpotSize() const
{
    Q_D(const Output);
    return d->hotSpotSize;
}

void Output::setHotSpotSize(const QSize &size)
{
    Q_D(Output);

    if (d->hotSpotSize == size)
        return;

    d->hotSpotSize = size;
    Q_EMIT hotSpotSizeChanged();
}

quint64 Output::hotSpotThreshold() const
{
    Q_D(const Output);
    return d->hotSpotThreshold;
}

void Output::setHotSpotThreshold(quint64 value)
{
    Q_D(Output);

    if (d->hotSpotThreshold == value)
        return;

    d->hotSpotThreshold = value;
    Q_EMIT hotSpotThresholdChanged();
}

quint64 Output::hotSpotPushTime() const
{
    Q_D(const Output);
    return d->hotSpotPushTime;
}

void Output::setHotSpotPushTime(quint64 value)
{
    Q_D(Output);

    if (d->hotSpotThreshold == value)
        return;

    d->hotSpotPushTime = value;
    Q_EMIT hotSpotPushTimeChanged();
}

OutputWindow *Output::window() const
{
    Q_D(const Output);
    return d->window;
}

QList<Surface *> Output::surfaces() const
{
    QList<Surface *> list;
    Q_FOREACH (Surface *surface, Compositor::instance()->surfaces()) {
        if (surface->outputs().contains(const_cast<Output *>(this)))
            list.append(surface);
    }

    return list;
}

Output *Output::fromResource(wl_resource *resource)
{
    OutputPrivate::Resource *outputResource =
            OutputPrivate::Resource::fromResource(resource);
    if (!outputResource)
        return Q_NULLPTR;

    OutputPrivate *output =
            static_cast<OutputPrivate *>(outputResource->output_object);
    if (!output)
        return Q_NULLPTR;

    return output->q_func();
}

QPointF Output::mapToOutput(const QPointF &pt)
{
    QPointF pos(geometry().topLeft());
    return QPointF(pt.x() - pos.x(), pt.y() - pos.y());
}

QPointF Output::mapToGlobal(const QPointF &pt)
{
    return geometry().topLeft() + pt;
}

} // namespace GreenIsland

#include "moc_output.cpp"
