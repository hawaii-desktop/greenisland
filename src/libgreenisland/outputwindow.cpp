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

#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>

#include "compositor.h"
#include "compositor_p.h"
#include "gldebug.h"
#include "globalregistry.h"
#include "logging.h"
#include "output.h"
#include "outputwindow.h"
#include "windowview.h"

#include "protocols/fullscreen-shell/fullscreenshellclient.h"
#include "protocols/greenisland/greenislandrecorder.h"

namespace GreenIsland {

OutputWindow::OutputWindow(Output *output)
    : QQuickView()
    , m_output(output)
    , m_hotSpotLastTime(0)
    , m_hotSpotEntered(0)
{
    // Initialize timer
    m_timer.start();

    // Setup window
    setColor(Qt::black);

    // Print GL information
    connect(this, SIGNAL(sceneGraphInitialized()),
            this, SLOT(printInfo()),
            Qt::DirectConnection);

    // Read content after rendering
    connect(this, &QQuickView::afterRendering,
            this, &OutputWindow::readContent,
            Qt::DirectConnection);

    // Retrieve full screen shell client object, this will be available
    // only when Green Island is nested into another compositor
    // that supports the fullscreen-shell interface
    if (QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        FullScreenShellClient *fsh = GlobalRegistry::fullScreenShell();
        if (fsh) {
            // Disable decorations
            setFlags(flags() | Qt::BypassWindowManagerHint);

            // Present output to full screen shell
            connect(this, &QQuickView::visibleChanged, this, [this, fsh](bool arg) {
                if (arg) {
                    qCDebug(GREENISLAND_COMPOSITOR)
                            << "Showing output on full screen shell for output"
                            << m_output->name() << m_output->geometry();
                    fsh->showOutput(m_output);
                } else {
                    qCDebug(GREENISLAND_COMPOSITOR)
                            << "Hiding output on full screen shell for output"
                            << m_output->name() << m_output->geometry();
                    fsh->hideOutput(m_output);
                }
            });
        }
    }

    // Show loading errors
    connect(this, &QQuickView::statusChanged, this, [this](const QQuickView::Status &status) {
        switch (status) {
        case QQuickView::Null:
            qCWarning(GREENISLAND_COMPOSITOR) << "No source set yet";
            break;
        case QQuickView::Loading:
            qCDebug(GREENISLAND_COMPOSITOR) << "Loading QML scene...";
            break;
        case QQuickView::Ready:
            qCDebug(GREENISLAND_COMPOSITOR) << "QML scene loaded successfully";
            break;
        default:
            qCWarning(GREENISLAND_COMPOSITOR) << "One or more errors have occurred loading the plugin:";
            Q_FOREACH (const QQmlError &error, errors())
                qCWarning(GREENISLAND_COMPOSITOR) << "*" << error.toString();
            break;
        }

        qCDebug(GREENISLAND_COMPOSITOR) << "Scene load time:" << m_perfTimer.elapsed() << "ms";
    });
}

OutputWindow::~OutputWindow()
{
    unloadScene();
}

Output *OutputWindow::output() const
{
    return m_output;
}

void OutputWindow::loadScene()
{
    // Show window
    if (!isVisible()) {
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Showing window for output"
                << m_output->name() << m_output->geometry();
        show();
    }

    qCDebug(GREENISLAND_COMPOSITOR)
            << "Loading scene on output"
            << m_output->name() << m_output->geometry();

    // Make compositor instance available to QML
    rootContext()->setContextProperty("compositor", m_output->compositor());

    // Add a context property to reference this window
    rootContext()->setContextProperty("_greenisland_window", this);

    // Add a context property to reference the output
    rootContext()->setContextProperty("_greenisland_output", m_output);

    // Load QML and setup window
    setResizeMode(QQuickView::SizeRootObjectToView);
    if (Compositor::s_fixedShell.isEmpty()) {
        qFatal("No plugin specified, cannot continue!");
    } else {
        // Load main file or bail out
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Loading" << Compositor::s_fixedShell
                << "shell for output"
                << m_output->name() << m_output->geometry();

        m_perfTimer.start();

        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QString("greenisland/%1/Compositor.qml").arg(Compositor::s_fixedShell));
        if (QFile(path).exists(path))
            setSource(QUrl::fromLocalFile(path));
        else
            qFatal("Shell \"%s\" is not valid, cannot continue!",
                   qPrintable(Compositor::s_fixedShell));
    }
}

void OutputWindow::unloadScene()
{
    // Unload the QML scene
    qCDebug(GREENISLAND_COMPOSITOR)
            << "Unloading scene from output"
            << m_output->name() << m_output->geometry();
    setSource(QUrl());

    // Hide window
    if (isVisible()) {
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Hiding window for output"
                << m_output->name() << m_output->geometry();
        hide();
    }
}

void OutputWindow::keyPressEvent(QKeyEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickView::keyPressEvent(event);
}

void OutputWindow::keyReleaseEvent(QKeyEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickView::keyReleaseEvent(event);
}

void OutputWindow::mousePressEvent(QMouseEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickView::mousePressEvent(event);
}

void OutputWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickView::mouseReleaseEvent(event);
}

void OutputWindow::mouseMoveEvent(QMouseEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    if (m_output) {
        const QPoint pt = event->localPos().toPoint();

        // Hot spots
        handleMotion(m_timer.elapsed(), pt);

        // Pointer barrier
        if (!m_output->geometry().contains(pt)) {
            event->ignore();
            return;
        }
    }

    QQuickView::mouseMoveEvent(event);
}

void OutputWindow::wheelEvent(QWheelEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickView::wheelEvent(event);
}

void OutputWindow::handleMotion(quint64 time, const QPoint &pt)
{
    // Can't continue without an output
    if (!m_output)
        return;

    // Hot spots are triggered after the configured threshold
    if (time - m_hotSpotLastTime < m_output->hotSpotThreshold())
        return;

    // Determine which hotspot was triggered
    int width = m_output->hotSpotSize().width();
    int height = m_output->hotSpotSize().height();
    Output::HotSpot hotSpot;
    bool triggered = true;
    if (pt.x() <= m_output->geometry().x() + width && pt.y() <= m_output->geometry().y() + height)
        hotSpot = Output::HotSpot::TopLeftHotSpot;
    else if (pt.x() >= m_output->geometry().right() - width && pt.y() <= m_output->geometry().y() + height)
        hotSpot = Output::HotSpot::TopRightHotSpot;
    else if (pt.x() <= m_output->geometry().x() + width && pt.y() >= m_output->geometry().bottom() - height)
        hotSpot = Output::HotSpot::BottomLeftHotSpot;
    else if (pt.x() >= m_output->geometry().right() - width && pt.y() >= m_output->geometry().bottom() - height)
        hotSpot = Output::HotSpot::BottomRightHotSpot;
    else {
        triggered = false;
        m_hotSpotEntered = 0;
    }

    // Trigger an action
    if (triggered) {
        if (m_hotSpotEntered == 0)
            m_hotSpotEntered = time;
        else if (time - m_hotSpotEntered > m_output->hotSpotPushTime()) {
            m_hotSpotLastTime = time;
            Q_EMIT m_output->hotSpotTriggered(hotSpot);
        }
    }
}

void OutputWindow::printInfo()
{
    GreenIsland::printGraphicsInformation(this);
}

void OutputWindow::readContent()
{
    // Send frame callbacks for windows rendered on this output
    m_output->sendFrameCallbacks();

    // Record a frame after rendering
    m_output->compositor()->d_ptr->recorderManager->recordFrame(this);
}

}

#include "moc_outputwindow.cpp"
