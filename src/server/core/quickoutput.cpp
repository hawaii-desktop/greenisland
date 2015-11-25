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

#include <QtCore/private/qobject_p.h>

#include "quickoutput.h"
#include "logging.h"
#include "screen/screenbackend.h"

namespace GreenIsland {

namespace Server {

/*
 * OutputPrivate
 */

class QuickOutputPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QuickOutput)
public:
    QuickOutputPrivate()
        : initialized(false)
        , nativeScreen(Q_NULLPTR)
        , hotSpotSize(QSize(10, 10))
        , hotSpotThreshold(1000)
        , hotSpotPushTime(150)
    {
    }

    bool initialized;
    Screen *nativeScreen;
    QSize hotSpotSize;
    quint64 hotSpotThreshold;
    quint64 hotSpotPushTime;
};

/*
 * Output
 */

QuickOutput::QuickOutput()
    : QWaylandQuickOutput()
    , d_ptr(new QuickOutputPrivate())
{
}

QuickOutput::QuickOutput(QWaylandCompositor *compositor)
    : QWaylandQuickOutput(compositor, Q_NULLPTR)
    , d_ptr(new QuickOutputPrivate())
{
}

Screen *QuickOutput::nativeScreen() const
{
    Q_D(const QuickOutput);
    return d->nativeScreen;
}

void QuickOutput::setNativeScreen(Screen *screen)
{
    Q_D(QuickOutput);

    if (d->nativeScreen == screen)
        return;

    if (d->initialized) {
        qCWarning(GREENISLAND_COMPOSITOR)
                << "Setting GreenIsland::Server::Output::nativeScreen "
                   "after initialization has no effect";
        return;
    }

    d->nativeScreen = screen;
    Q_EMIT nativeScreenChanged();
}

QSize QuickOutput::hotSpotSize() const
{
    Q_D(const QuickOutput);
    return d->hotSpotSize;
}

void QuickOutput::setHotSpotSize(const QSize &size)
{
    Q_D(QuickOutput);

    if (d->hotSpotSize == size)
        return;

    d->hotSpotSize = size;
    Q_EMIT hotSpotSizeChanged();
}

quint64 QuickOutput::hotSpotThreshold() const
{
    Q_D(const QuickOutput);
    return d->hotSpotThreshold;
}

void QuickOutput::setHotSpotThreshold(quint64 value)
{
    Q_D(QuickOutput);

    if (d->hotSpotThreshold == value)
        return;

    d->hotSpotThreshold = value;
    Q_EMIT hotSpotThresholdChanged();
}

quint64 QuickOutput::hotSpotPushTime() const
{
    Q_D(const QuickOutput);
    return d->hotSpotPushTime;
}

void QuickOutput::setHotSpotPushTime(quint64 value)
{
    Q_D(QuickOutput);

    if (d->hotSpotThreshold == value)
        return;

    d->hotSpotPushTime = value;
    Q_EMIT hotSpotPushTimeChanged();
}

void QuickOutput::initialize()
{
    Q_D(QuickOutput);

    QWaylandQuickOutput::initialize();

    QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(window());
    if (!quickWindow) {
        qCWarning(GREENISLAND_COMPOSITOR,
                  "Could not locate QQuickWindow on initializing"
                  "GreenIsland::Server::Output %p.\n", this);
        return;
    }

    // By default windows use the primary screen, but this will make
    // the compositor fail when using the eglfs or greenisland QPA plugins
    // because they both need one window for each screen.
    // Avoid failures by setting the screen from the native screen
    // that comes from our screen manager.
    if (d->nativeScreen)
        quickWindow->setScreen(d->nativeScreen->screen());

    // We want to read contents to make a screenshot
    connect(quickWindow, &QQuickWindow::afterRendering,
            this, &QuickOutput::readContent,
            Qt::DirectConnection);

    // Set the window visible now
    quickWindow->setVisible(true);

    d->initialized = true;
}

void QuickOutput::readContent()
{
    // TODO: Update the screencaster protocol with the new API
    // and record a frame here
}

} // namespace Server

} // namespace GreenIsland

#include "moc_quickoutput.cpp"
