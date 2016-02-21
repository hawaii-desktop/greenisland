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

#include <QtCore/private/qobject_p.h>
#include <QtGui/QCursor>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QPixmap>

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>

#include "globalpointertracker.h"

class GlobalPointerTrackerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(GlobalPointerTracker)
public:
    GlobalPointerTrackerPrivate()
        : enableSystemPointer(true)
        , compositor(Q_NULLPTR)
        , output(Q_NULLPTR)
    {
        QImage cursorImage(64, 64, QImage::Format_ARGB32);
        cursorImage.fill(Qt::transparent);
        emptyCursorPixmap = QPixmap::fromImage(cursorImage);

        if (!enableSystemPointer)
            QGuiApplication::setOverrideCursor(QCursor(emptyCursorPixmap));
    }

    void handleMousePos()
    {
        Q_Q(GlobalPointerTracker);

        if (!compositor)
            return;

        Q_FOREACH (QWaylandOutput *o, compositor->outputs()) {
            if (o->geometry().contains(pos.toPoint())) {
                if (output != o) {
                    output = o;
                    Q_EMIT q->outputChanged();
                }
                break;
            }
        }
    }

    QPixmap emptyCursorPixmap;
    bool enableSystemPointer;
    QPointF pos;
    QWaylandCompositor *compositor;
    QWaylandOutput *output;
};

GlobalPointerTracker::GlobalPointerTracker(QObject *parent)
    : QObject(*new GlobalPointerTrackerPrivate(), parent)
{
}

QWaylandCompositor *GlobalPointerTracker::compositor() const
{
    Q_D(const GlobalPointerTracker);
    return d->compositor;
}

void GlobalPointerTracker::setCompositor(QWaylandCompositor *compositor)
{
    Q_D(GlobalPointerTracker);

    if (d->compositor == compositor)
        return;

    d->compositor = compositor;
    Q_EMIT compositorChanged();
}

bool GlobalPointerTracker::isSystemPointerEnabled() const
{
    Q_D(const GlobalPointerTracker);
    return d->enableSystemPointer;
}

void GlobalPointerTracker::setSystemPointerEnabled(bool enable)
{
    Q_D(GlobalPointerTracker);

    if (d->enableSystemPointer == enable)
        return;

    d->enableSystemPointer = enable;

    if (enable)
        QGuiApplication::restoreOverrideCursor();
    else
        QGuiApplication::setOverrideCursor(QCursor(d->emptyCursorPixmap));

    Q_EMIT enableSystemPointerChanged();
}

qreal GlobalPointerTracker::mouseX() const
{
    Q_D(const GlobalPointerTracker);
    return d->pos.x();
}

void GlobalPointerTracker::setMouseX(qreal x)
{
    Q_D(GlobalPointerTracker);

    if (d->pos.x() == x)
        return;

    d->pos.setX(x);
    d->handleMousePos();
    Q_EMIT xChanged();
}

qreal GlobalPointerTracker::mouseY() const
{
    Q_D(const GlobalPointerTracker);
    return d->pos.y();
}

void GlobalPointerTracker::setMouseY(qreal y)
{
    Q_D(GlobalPointerTracker);

    if (d->pos.y() == y)
        return;

    d->pos.setY(y);
    d->handleMousePos();
    Q_EMIT yChanged();
}

QWaylandOutput *GlobalPointerTracker::output() const
{
    Q_D(const GlobalPointerTracker);
    return d->output;
}

#include "moc_globalpointertracker.cpp"
