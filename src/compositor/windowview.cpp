/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandOutput>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/private/qwloutput_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "compositor.h"
#include "quicksurface.h"
#include "windowview.h"

namespace GreenIsland {

WindowView::WindowView(QuickSurface *surface, Output *output, QQuickItem *parent)
    : QWaylandSurfaceItem(surface, parent)
    , m_surface(surface)
    , m_output(output)
{
    // Change window position and send enter/leave events to the output
    connect(m_surface, &QuickSurface::globalGeometryChanged, [=]() {
        // WindowView is a child of the QtQuick window representation that is
        // the one who holds the position on screen
        if (parentItem())
            parentItem()->setPosition(m_output->mapToOutput(m_surface->globalPosition()));
        else
            qWarning("Unable to move this view because it has no window representation");

        // Global position is changed every time the window is moved,
        // check against output geometry to see if it enters or leave
        // the output
        if (QRectF(m_output->geometry()).intersects(m_surface->globalGeometry()))
            sendEnter(m_output);
        else
            sendLeave(m_output);
    });
}

QuickSurface *WindowView::surface() const
{
    return m_surface;
}

Output *WindowView::output() const
{
    return m_output;
}

Output *WindowView::mainOutput() const
{
    // Find the output that contains the biggest part of this window,
    // that is the main output and it will be used by effects such as
    // present windows to present only windows for the output it is
    // running on (effects run once for each output)
    QRectF geometry(m_surface->globalPosition(), QSizeF(width(), height()));
    int maxArea = 0, area = 0;
    QWaylandOutput *main = Q_NULLPTR;

    for (QWaylandOutput *output: compositor()->outputs()) {
        QRectF intersection = QRectF(output->geometry()).intersected(geometry);

        if (intersection.isValid()) {
            area = intersection.width() * intersection.height();
            if (area >= maxArea) {
                main = output;
                maxArea = area;
            }
        }
    }

    return qobject_cast<Output *>(main);
}

void WindowView::mouseReleaseEvent(QMouseEvent *event)
{
    QWaylandSurfaceItem::mouseReleaseEvent(event);

    if (surface()) {
        // Raise window when clicked, whether to assign focus
        // is decided from QML
        Q_EMIT surface()->raiseRequested();
    }
}

void WindowView::sendEnter(Output *output)
{
    //qDebug() << "Enter" << output;

    for (QtWayland::Output::Resource *resource: output->handle()->resourceMap().values())
        surface()->handle()->send_enter(resource->handle);
}

void WindowView::sendLeave(Output *output)
{
    //qDebug() << "Leave" << output;

    for (QtWayland::Output::Resource *resource: output->handle()->resourceMap().values())
        surface()->handle()->send_leave(resource->handle);
}

}

#include "moc_windowview.cpp"
