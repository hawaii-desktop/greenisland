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

#include "windowview.h"
#include "output.h"

WindowView::WindowView(QWaylandQuickSurface *surface, QWaylandOutput *output, QQuickItem *parent)
    : QWaylandSurfaceItem(surface, parent)
    , m_output(output)
{
#if 0
    // Update global geometry ourselves every time the parent has changed
    // TODO: All of this should be done only when the window is moved or resized by the user
    // so I leave it commented out for now
    connect(this, &QQuickItem::parentChanged, [=](QQuickItem *newParent) {
        if (!newParent)
            return;

        connect(newParent, &WindowView::xChanged, [=]() {
            m_globalGeometry.setX(newParent->position().x());
        });
        connect(newParent, &WindowView::yChanged, [=]() {
            m_globalGeometry.setY(newParent->position().y());
        });
        connect(newParent, &WindowView::widthChanged, [=]() {
            m_globalGeometry.setWidth(newParent->width());
        });
        connect(newParent, &WindowView::heightChanged, [=]() {
            m_globalGeometry.setHeight(newParent->height());
        });
    });
#endif
}

QWaylandOutput *WindowView::output() const
{
    return m_output;
}

QWaylandOutput *WindowView::mainOutput() const
{
    // Find the output that contains the biggest part of this window,
    // that is the main output and it will be used by effects such as
    // present windows to present only windows for the output it is
    // running on (effects run once for each output)
    int maxArea = 0, area = 0;
    QWaylandOutput *main = Q_NULLPTR;

    for (QWaylandOutput *output: compositor()->outputs()) {
        QRectF intersection = QRectF(output->geometry()).intersected(m_globalGeometry);

        if (intersection.isValid()) {
            area = intersection.width() * intersection.height();
            if (area >= maxArea) {
                main = output;
                maxArea = area;
            }
        }
    }

    return main;
}

QRectF WindowView::globalGeometry() const
{
    return m_globalGeometry;
}

void WindowView::setGlobalGeometry(const QRectF &g)
{
    if (m_globalGeometry == g)
        return;

    if (parentItem())
        parentItem()->setPosition(qobject_cast<Output *>(output())->mapToOutput(g.topLeft()));
    else
        qWarning() << "Unable to set global geometry because view" << this << "has no parent";

    // WindowView is a child to the QtQuick window representation,
    // when the latter is moved or resized it updates the globalGeometry
    // property with window geometry in global coordinate space.
    // We check globalGeometry against the output this view is in
    // to determine whether the window has entered or left the output
    if (QRectF(output()->geometry()).intersects(g))
        sendEnter(output());
    else
        sendLeave(output());

    m_globalGeometry = g;
    Q_EMIT globalGeometryChanged();
}

void WindowView::sendEnter(QWaylandOutput *output)
{
    //qDebug() << "Enter" << output;

    for (QtWayland::Output::Resource *resource: output->handle()->resourceMap().values())
        surface()->handle()->send_enter(resource->handle);
}

void WindowView::sendLeave(QWaylandOutput *output)
{
    //qDebug() << "Leave" << output;

    for (QtWayland::Output::Resource *resource: output->handle()->resourceMap().values())
        surface()->handle()->send_leave(resource->handle);
}

#include "moc_windowview.cpp"
