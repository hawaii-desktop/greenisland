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

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QQmlContext>
#include <QQuickItem>

#include <QtCompositor/wlsurface.h>
#include <QtCompositor/waylandsurface.h>
#include <QtCompositor/waylandsurfaceitem.h>
#include <QtCompositor/waylandinput.h>

#include "systemcompositor.h"
#include "cmakedirs.h"
#include "systemcompositorserver.h"
#include "systemclient.h"

SystemCompositor::SystemCompositor()
    : VCompositor(this)
    , m_currentSurface(0)
{
    // Enable the subsurface extension
    enableSubSurfaceExtension();

    // System compositor protocol
    m_protocol = new SystemCompositorServer(this, WaylandCompositor::handle());

    // Allow QML to access this compositor
    rootContext()->setContextProperty("compositor", this);

    // All the screen is initially available
    m_availableGeometry = screen()->availableGeometry();
    connect(screen(), SIGNAL(virtualGeometryChanged(QRect)),
            this, SIGNAL(screenGeometryChanged()));

    // Load the QML code
    setSource(QUrl("qrc:///qml/Compositor.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::black);
    winId();

    connect(this, SIGNAL(windowAdded(QVariant)),
            rootObject(), SLOT(windowAdded(QVariant)));
    connect(this, SIGNAL(windowDestroyed(QVariant)),
            rootObject(), SLOT(windowDestroyed(QVariant)));
    connect(this, SIGNAL(windowResized(QVariant)),
            rootObject(), SLOT(windowResized(QVariant)));
    connect(this, SIGNAL(sceneGraphInitialized()),
            this, SLOT(sceneGraphInitialized()), Qt::DirectConnection);
    connect(this, SIGNAL(frameSwapped()),
            this, SLOT(frameSwapped()));
}

QRectF SystemCompositor::screenGeometry() const
{
    return screen()->availableGeometry();
}

QRectF SystemCompositor::availableGeometry() const
{
    return m_availableGeometry;
}

void SystemCompositor::setAvailableGeometry(const QRectF &g)
{
    m_availableGeometry = g;
    emit availableGeometryChanged();
}

SystemClient *SystemCompositor::systemClientForClient(WaylandClient *client)
{
    for (int i = 0; i < m_systemClients.size(); i++) {
        SystemClient *systemClient = m_systemClients.at(i);

        if (systemClient->client() == client)
            return systemClient;
    }

    return 0;
}

void SystemCompositor::surfaceCreated(WaylandSurface *surface)
{
    // Connect surface signals
    connect(surface, SIGNAL(destroyed(QObject *)),
            this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(mapped()),
            this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()),
            this, SLOT(surfaceUnmapped()));
}

void SystemCompositor::surfaceAboutToBeDestroyed(WaylandSurface *surface)
{
    // TODO:
}

void SystemCompositor::destroyWindow(QVariant window)
{
    qvariant_cast<QObject *>(window)->deleteLater();
}

void SystemCompositor::destroyClientForWindow(QVariant window)
{
    WaylandSurface *surface = qobject_cast<WaylandSurfaceItem *>(
                                  qvariant_cast<QObject *>(window))->surface();
    destroyClientForSurface(surface);
}

void SystemCompositor::setCurrentSurface(WaylandSurface *surface)
{
    if (surface == m_currentSurface)
        return;
    m_currentSurface = surface;
    emit currentSurfaceChanged();
}

void SystemCompositor::surfaceMapped()
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());

    //A surface without a shell surface is not a window
    if (!surface->hasShellSurface())
        return;

    WaylandSurfaceItem *item = surface->surfaceItem();

    // Create a WaylandSurfaceItem from the surface
    if (!item) {
        item = new WaylandSurfaceItem(surface, rootObject());
        item->setClientRenderingEnabled(true);
        item->setTouchEventsEnabled(true);
    }

    // Surface items gain focus right after they were mapped
    item->takeFocus();

    // Announce a window was added
    emit windowAdded(QVariant::fromValue(static_cast<QQuickItem *>(item)));
}

void SystemCompositor::surfaceUnmapped()
{
    // Set to 0 the current surface if it was unmapped
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    emit windowDestroyed(QVariant::fromValue(item));
}

void SystemCompositor::surfaceDestroyed(QObject *object)
{
    // Set to 0 the current surface if it was destroyed
    WaylandSurface *surface = static_cast<WaylandSurface *>(object);
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    if (item)
        emit windowDestroyed(QVariant::fromValue(item));
}

void SystemCompositor::sceneGraphInitialized()
{
    showGraphicsInfo();
}

void SystemCompositor::frameSwapped()
{
    frameFinished(m_currentSurface);
}

void SystemCompositor::resizeEvent(QResizeEvent *event)
{
    // Scale compositor output to window's size
    QQuickView::resizeEvent(event);
    WaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

#include "moc_systemcompositor.cpp"
