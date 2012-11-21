/****************************************************************************
 * This file is part of Green Island.
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

Compositor::Compositor()
    : VCompositor(this)
    , m_currentSurface(0)
    , m_shellProcess(0)
{
    // Desktop shell protocol
    m_desktopShell = new DesktopShellServer(WaylandCompositor::handle());

    // Load the QML code
    enableSubSurfaceExtension();
    setSource(QUrl("qrc:///qml/Compositor.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::black);
    winId();

    // All the screen is available
    m_availableGeometry = geometry();

    // Allow QML to access this compositor
    rootContext()->setContextProperty("compositor", this);

    connect(this, SIGNAL(windowAdded(QVariant)),
            rootObject(), SLOT(windowAdded(QVariant)));
    connect(this, SIGNAL(windowDestroyed(QVariant)),
            rootObject(), SLOT(windowDestroyed(QVariant)));
    connect(this, SIGNAL(windowResized(QVariant)),
            rootObject(), SLOT(windowResized(QVariant)));
    connect(this, SIGNAL(frameSwapped()),
            this, SLOT(frameSwapped()));
}

Compositor::~Compositor()
{
    m_shellProcess->close();
    delete m_shellProcess;
}

void Compositor::runShell()
{
    // Force Wayland as a QPA plugin and GTK+ backend and reuse XDG_RUNTIME_DIR
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QLatin1String("EGL_PLATFORM"), QLatin1String("wayland"));
    env.remove(QLatin1String("QT_QPA_GENERIC_PLUGINS"));
    env.insert(QLatin1String("QT_QPA_PLATFORM"), QLatin1String("wayland"));
    env.insert(QLatin1String("GDK_BACKEND"), QLatin1String("wayland"));
    env.insert(QLatin1String("XDG_RUNTIME_DIR"), qgetenv("XDG_RUNTIME_DIR"));

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

QRectF Compositor::availableGeometry() const
{
    return m_availableGeometry;
}

void Compositor::setAvailableGeometry(const QRectF &rect)
{
    m_availableGeometry = rect;
    emit availableGeometryChanged();
}

void Compositor::surfaceCreated(WaylandSurface *surface)
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

void Compositor::surfaceAboutToBeDestroyed(WaylandSurface *surface)
{
    // TODO:
}

void Compositor::destroyWindow(QVariant window)
{
    qvariant_cast<QObject *>(window)->deleteLater();
}

void Compositor::destroyClientForWindow(QVariant window)
{
    WaylandSurface *surface = qobject_cast<WaylandSurfaceItem *>(
                                  qvariant_cast<QObject *>(window))->surface();
    destroyClientForSurface(surface);
}

void Compositor::setCurrentSurface(WaylandSurface *surface)
{
    if (surface == m_currentSurface)
        return;
    m_currentSurface = surface;
    emit currentSurfaceChanged();
}

void Compositor::shellStarted()
{
    if (m_shellProcess)
        qDebug() << "Shell is ready!";
}

void Compositor::shellFailed(QProcess::ProcessError error)
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

void Compositor::shellReadyReadStandardOutput()
{
    if (m_shellProcess)
        printf("shell: %s", m_shellProcess->readAllStandardOutput().constData());
}

void Compositor::shellReadyReadStandardError()
{
    if (m_shellProcess)
        fprintf(stderr, "shell: %s", m_shellProcess->readAllStandardError().constData());
}

void Compositor::shellAboutToClose()
{
    qDebug() << "Shell is about to close...";
}

void Compositor::surfaceMapped()
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());

    // Surface items gain focus right after they were mapped
    WaylandSurfaceItem *item = surface->surfaceItem();
    item->takeFocus();

    // Announce a window was added
    emit windowAdded(QVariant::fromValue(static_cast<QQuickItem *>(item)));
}

void Compositor::surfaceUnmapped()
{
    // Set to 0 the current surface if it was unmapped
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    emit windowDestroyed(QVariant::fromValue(item));
}

void Compositor::surfaceDestroyed(QObject *object)
{
    // Set to 0 the current surface if it was destroyed
    WaylandSurface *surface = static_cast<WaylandSurface *>(object);
    if (surface == m_currentSurface)
        m_currentSurface = 0;

    // Announce this window was destroyed
    QQuickItem *item = surface->surfaceItem();
    emit windowDestroyed(QVariant::fromValue(item));
}

void Compositor::frameSwapped()
{
    frameFinished(m_currentSurface);
}

void Compositor::resizeEvent(QResizeEvent *event)
{
    // Scale compositor output to window's size
    QQuickView::resizeEvent(event);
    WaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

#include "moc_compositor.cpp"
