/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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

#include <QGuiApplication>
#include <QScreen>
#include <QQmlContext>
#include <QQuickItem>
#include <QtCompositor/waylandsurface.h>
#include <QtCompositor/waylandsurfaceitem.h>
#include <QtCompositor/waylandinput.h>

#include "desktopshell.h"
#include "cmakedirs.h"

DesktopShell::DesktopShell()
    : VShell(this)    
    , m_currentSurface(0)
{
    // Load the shell
    enableSubSurfaceExtension();
    setSource(QUrl("qrc:///qml/Shell.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::black);
    winId();

    // All the screen is available
    m_availableGeometry = geometry();

    // Allow QML to access this shell
    rootContext()->setContextProperty("shell", this);

    connect(this, SIGNAL(windowAdded(QVariant)),
        rootObject(), SLOT(windowAdded(QVariant)));
    connect(this, SIGNAL(windowDestroyed(QVariant)),
        rootObject(), SLOT(windowDestroyed(QVariant)));
    connect(this, SIGNAL(windowResized(QVariant)),
        rootObject(), SLOT(windowResized(QVariant)));
    connect(this, SIGNAL(frameSwapped()),
        this, SLOT(frameSwapped()));
}

QRectF DesktopShell::availableGeometry() const
{
    return m_availableGeometry;
}

void DesktopShell::setAvailableGeometry(const QRectF &rect)
{
    m_availableGeometry = rect;
    emit availableGeometryChanged();
}

void DesktopShell::setupCompositor()
{
    setWindowTitle("Green Island");
    setGeometry(QGuiApplication::primaryScreen()->availableGeometry());
    showMaximized();
}

void DesktopShell::startShell()
{
    // Set path so that programs will be found
    QByteArray path = qgetenv("PATH");
    if (!path.isEmpty())
        path += ":";
    path += INSTALL_BINDIR;
    setenv("PATH", qPrintable(path), 1);

    // Force QPA to wayland
    setenv("QT_QPA_PLATFORM", "wayland", 1);

    // Force GTK+ backend to wayland
    setenv("GDK_BACKEND", "wayland", 1);
}

void DesktopShell::surfaceCreated(WaylandSurface *surface)
{
    // Create a WaylandSurfaceItem from the surface
    WaylandSurfaceItem *item = new WaylandSurfaceItem(surface, rootObject());
    item->setClientRenderingEnabled(true);
    item->setTouchEventsEnabled(true);

    // Connect surface signals
    connect(surface, SIGNAL(destroyed(QObject *)),
        this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(mapped()),
        this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()),
        this, SLOT(surfaceUnmapped()));
}

void DesktopShell::surfaceAboutToBeDestroyed(WaylandSurface *surface)
{
    // TODO:
}

void DesktopShell::destroyWindow(QVariant window)
{
    qvariant_cast<QObject *>(window)->deleteLater();
}

void DesktopShell::destroyClientForWindow(QVariant window)
{
    WaylandSurface *surface = qobject_cast<WaylandSurfaceItem *>(
        qvariant_cast<QObject *>(window))->surface();
    destroyClientForSurface(surface);
}

void DesktopShell::setCurrentSurface(WaylandSurface *surface)
{
    if (surface == m_currentSurface)
        return;
    m_currentSurface = surface;
    emit currentSurfaceChanged();
}

void DesktopShell::surfaceMapped()
{
    // Surface items gain focus right after they were mapped
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    WaylandSurfaceItem *item = surface->surfaceItem();
    item->takeFocus();

    // Announce a window was added
    emit windowAdded(QVariant::fromValue(static_cast<QQuickItem *>(item)));
}

void DesktopShell::surfaceUnmapped()
{
    // Set to 0 the current surface if it was unmapped
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    emit windowDestroyed(QVariant::fromValue(item));
}

void DesktopShell::surfaceDestroyed(QObject *object)
{
    // Set to 0 the current surface if it was destroyed
    WaylandSurface *surface = static_cast<WaylandSurface *>(object);
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    emit windowDestroyed(QVariant::fromValue(item));
}

void DesktopShell::frameSwapped()
{
    frameFinished(m_currentSurface);
}

void DesktopShell::resizeEvent(QResizeEvent *event)
{
    // Scale compositor output to window's size
    QQuickView::resizeEvent(event);
    WaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

#include "moc_desktopshell.cpp"
