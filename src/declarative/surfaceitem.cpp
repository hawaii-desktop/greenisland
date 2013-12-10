/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QTimer>

#include "surfaceitem.h"

class SurfaceItemPrivate
{
    Q_DECLARE_PUBLIC(SurfaceItem)
public:
    SurfaceItemPrivate(SurfaceItem *parent)
        : q_ptr(parent)
        , timer(new QTimer(parent))
        , unresponsive(false)
    {
        timer->setInterval(200);
        QObject::connect(timer, &QTimer::timeout, [=]() {
            Q_Q(SurfaceItem);

            if (!unresponsive) {
                unresponsive = true;
                Q_EMIT q->unresponsiveChanged(true);
            }
        });
    }

    void pingSurface()
    {
        Q_Q(SurfaceItem);

        // Ping the surface to see whether it's responsive, if a pong
        // doesn't arrive before pingTimeout is trigger we know the
        // surface is unresponsive and mark the container's flag
        if (q->surface() && q->paintEnabled()) {
            QObject::connect(q->surface(), &QWaylandSurface::pong, [=]() {
                timer->stop();

                if (unresponsive) {
                    Q_Q(SurfaceItem);
                    unresponsive = false;
                    Q_EMIT q->unresponsiveChanged(false);
                }
            });

            q->surface()->ping();
            timer->start();
        }
    }

    SurfaceItem *const q_ptr;
    QTimer *timer;
    bool unresponsive;
};

SurfaceItem::SurfaceItem(QQuickItem *parent)
    : QWaylandSurfaceItem(parent)
    , d_ptr(new SurfaceItemPrivate(this))
{
}

SurfaceItem::SurfaceItem(QWaylandSurface *surface, QQuickItem *parent)
    : QWaylandSurfaceItem(surface, parent)
    , d_ptr(new SurfaceItemPrivate(this))
{
}

SurfaceItem::~SurfaceItem()
{
    delete d_ptr;
}

bool SurfaceItem::isUnresponsive() const
{
    Q_D(const SurfaceItem);
    return d->unresponsive;
}

void SurfaceItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(SurfaceItem);

    if (surface() && paintEnabled()) {
        // Give focus to this surface when the mouse is pressed
        takeFocus();

        // Check whether the surface is responsive or not
        d->pingSurface();
    }

    // Continue with normal event handling
    QWaylandSurfaceItem::mousePressEvent(event);
}

void SurfaceItem::touchEvent(QTouchEvent *event)
{
    Q_D(SurfaceItem);

    if (surface() && paintEnabled() && touchEventsEnabled()) {
        if (event->type() == QEvent::TouchBegin) {
            // Give focus to this surface at every tap on the surface
            takeFocus();

            // Check whether the surface is responsive or not
            d->pingSurface();
        }
    }

    // Continue with normal event handling
    QWaylandSurfaceItem::touchEvent(event);
}

#include "moc_surfaceitem.cpp"
