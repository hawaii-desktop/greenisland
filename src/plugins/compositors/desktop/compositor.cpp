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

#include "compositor.h"
#include "desktopshellserver.h"
#include "cmakedirs.h"

DesktopCompositor::DesktopCompositor()
    : VCompositor(this)
    , m_shellSurface(0)
    , m_currentSurface(0)
    , m_shellProcess(0)
{
    // Enable the subsurface extension
    enableSubSurfaceExtension();

    // Desktop shell protocol
    m_desktopShell = new DesktopShellServer(this, WaylandCompositor::handle());

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

void DesktopCompositor::runShell()
{
    // Force Wayland as a QPA plugin and GTK+ backend
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QLatin1String("EGL_PLATFORM"), QLatin1String("wayland"));
    env.remove(QLatin1String("QT_QPA_GENERIC_PLUGINS"));
    env.insert(QLatin1String("QT_QPA_PLATFORM"), QLatin1String("wayland"));
    env.insert(QLatin1String("GDK_BACKEND"), QLatin1String("wayland"));

    // Process arguments
    QStringList arguments = QStringList()
                            << QLatin1String("-platform") << QLatin1String("wayland");

    // Run the shell client process
    m_shellProcess = new QProcess(this);
    connect(m_shellProcess, SIGNAL(started()),
            this, SLOT(shellStarted()));
    connect(m_shellProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(shellFailed(QProcess::ProcessError)));
    connect(m_shellProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(shellReadyReadStandardOutput()));
    connect(m_shellProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(shellReadyReadStandardError()));
    connect(m_shellProcess, SIGNAL(aboutToClose()),
            this, SLOT(shellAboutToClose()));
    m_shellProcess->setProcessEnvironment(env);
    m_shellProcess->start(QLatin1String(INSTALL_LIBEXECDIR "/greenisland-desktop-shell"),
                          arguments, QIODevice::ReadOnly);
}

void DesktopCompositor::closeShell()
{
    if (m_shellSurface)
        destroyClientForSurface(m_shellSurface);
    m_shellProcess->close();
    delete m_shellProcess;
}

QRectF DesktopCompositor::screenGeometry() const
{
    return screen()->availableGeometry();
}

QRectF DesktopCompositor::availableGeometry() const
{
    return m_availableGeometry;
}

void DesktopCompositor::setAvailableGeometry(const QRectF &g)
{
    m_availableGeometry = g;
    emit availableGeometryChanged();
}

void DesktopCompositor::surfaceCreated(WaylandSurface *surface)
{
    // Connect surface signals
    connect(surface, SIGNAL(destroyed(QObject *)),
            this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(mapped()),
            this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()),
            this, SLOT(surfaceUnmapped()));
}

void DesktopCompositor::surfaceAboutToBeDestroyed(WaylandSurface *surface)
{
    // TODO:
}

void DesktopCompositor::destroyWindow(QVariant window)
{
    qvariant_cast<QObject *>(window)->deleteLater();
}

void DesktopCompositor::destroyClientForWindow(QVariant window)
{
    WaylandSurface *surface = qobject_cast<WaylandSurfaceItem *>(
                                  qvariant_cast<QObject *>(window))->surface();
    destroyClientForSurface(surface);
}

void DesktopCompositor::setCurrentSurface(WaylandSurface *surface)
{
    if (surface == m_currentSurface)
        return;
    m_currentSurface = surface;
    emit currentSurfaceChanged();
}

void DesktopCompositor::shellStarted()
{
    if (m_shellProcess)
        qDebug() << "Shell is ready!";
}

void DesktopCompositor::shellFailed(QProcess::ProcessError error)
{
    switch (error) {
        case QProcess::FailedToStart:
            qWarning("The shell process failed to start.\n"
                     "Either the invoked program is missing, or you may have insufficient permissions to run it.");
            break;
        case QProcess::Crashed:
            qWarning("The shell process crashed some time after starting successfully.");
            break;
        case QProcess::Timedout:
            qWarning("The shell process timedout.\n");
            break;
        case QProcess::WriteError:
            qWarning("An error occurred when attempting to write to the shell process.");
            break;
        case QProcess::ReadError:
            qWarning("An error occurred when attempting to read from the shell process.");
            break;
        case QProcess::UnknownError:
            qWarning("Unknown error starting the shell process!");
            break;
    }

    // Don't need it anymore because it failed
    m_shellProcess->close();
    delete m_shellProcess;
    m_shellProcess = 0;
}

void DesktopCompositor::shellReadyReadStandardOutput()
{
    if (m_shellProcess)
        printf("%s", m_shellProcess->readAllStandardOutput().constData());
}

void DesktopCompositor::shellReadyReadStandardError()
{
    if (m_shellProcess)
        fprintf(stderr, "%s", m_shellProcess->readAllStandardError().constData());
}

void DesktopCompositor::shellAboutToClose()
{
    qDebug() << "Shell is about to close...";
}

void DesktopCompositor::surfaceMapped()
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

    // Save shell surface pointer
    if (surface->className() == QStringLiteral("greenisland-desktop-shell.desktop"))
        m_shellSurface = surface;

    // Surface items gain focus right after they were mapped
    item->takeFocus();

    // Announce a window was added
    emit windowAdded(QVariant::fromValue(static_cast<QQuickItem *>(item)));
}

void DesktopCompositor::surfaceUnmapped()
{
    // Set to 0 the current surface if it was unmapped
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    if (item)
        emit windowDestroyed(QVariant::fromValue(item));
}

void DesktopCompositor::surfaceDestroyed(QObject *object)
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

void DesktopCompositor::sceneGraphInitialized()
{
    showGraphicsInfo();
}

void DesktopCompositor::frameSwapped()
{
    frameFinished();
}

void DesktopCompositor::resizeEvent(QResizeEvent *event)
{
    // Scale compositor output to window's size
    QQuickView::resizeEvent(event);
    WaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

#include "moc_compositor.cpp"
