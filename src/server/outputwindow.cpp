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

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>

#include "compositor.h"
#include "compositor_p.h"
#include "logging.h"
#include "keybinding.h"
#include "keybindings.h"
#include "output.h"
#include "outputwindow.h"
#include "windowview.h"

#include "protocols/fullscreen-shell/fullscreenshellclient.h"
#include "protocols/greenisland/greenislandrecorder.h"

namespace GreenIsland {

OutputWindow::OutputWindow(Output *output)
    : QQuickWindow()
    , m_output(output)
    , m_component(Q_NULLPTR)
    , m_context(Q_NULLPTR)
    , m_hotSpotLastTime(0)
    , m_hotSpotEntered(0)
{
    // Initialize timer
    m_timer.start();

    // Setup window
    setColor(Qt::black);

    // Read content after rendering
    connect(this, &QQuickWindow::afterRendering,
            this, &OutputWindow::readContent,
            Qt::DirectConnection);
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
    qCDebug(GREENISLAND_COMPOSITOR)
            << "Loading scene on output"
            << m_output->name() << m_output->geometry();

    QString shell = Compositor::instance()->shell();

    // Load QML and setup window
    if (shell.isEmpty())
        qFatal("No shell specified, cannot continue!");

    // Load main file or bail out
    qCDebug(GREENISLAND_COMPOSITOR)
            << "Loading" << shell << "shell for output"
            << m_output->name() << m_output->geometry();

    m_perfTimer.start();

    // Pick the first shell directory we find
    QString mainScriptFileName;
    QStringList dirs =
            QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                      QStringLiteral("greenisland/shells/") + shell,
                                      QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &pathName, dirs) {
        QDir dir(pathName);
        QFileInfo info(dir, QStringLiteral("metadata.desktop"));
        if (info.exists()) {
            // Retrieve the QML file name
            QSettings shell(info.absoluteFilePath(), QSettings::IniFormat);
            shell.setIniCodec("UTF-8");
            shell.beginGroup(QStringLiteral("Shell"));
            mainScriptFileName = dir.absoluteFilePath(
                        shell.value(QStringLiteral("MainScript"),
                                    QStringLiteral("Compositor.qml")).toString());
            break;
        }
    }

    // Bail out if the QML file was not found
    if (mainScriptFileName.isEmpty())
        qFatal("Shell \"%s\" is not valid, cannot continue!",
               qPrintable(shell));

    // Engine
    QQmlEngine *engine = Compositor::instance()->engine();

    // Context
    if (!m_context) {
        m_context = new QQmlContext(engine->rootContext(), this);
        m_context->setContextProperty("_greenisland_window", this);
        m_context->setContextProperty("_greenisland_output", m_output);
    }

    // Component
    m_component = new QQmlComponent(engine, QUrl::fromLocalFile(mainScriptFileName));
    if (m_component->isLoading())
        connect(m_component, &QQmlComponent::statusChanged,
                this, &OutputWindow::statusChanged);
    else
        statusChanged(QQmlComponent::Ready);
}

void OutputWindow::unloadScene()
{
    // Unload the QML scene
    qCDebug(GREENISLAND_COMPOSITOR)
            << "Unloading scene from output"
            << m_output->name() << m_output->geometry();
    if (m_component) {
        disconnect(m_component, &QQmlComponent::statusChanged,
                   this, &OutputWindow::statusChanged);
        m_component->deleteLater();
    }
    m_component = Q_NULLPTR;

    // Hide window
    if (isVisible()) {
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Hiding window for output"
                << m_output->name() << m_output->geometry();
        hideOutput();
    }
}

void OutputWindow::keyPressEvent(QKeyEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    // Key bindings
    KeyBindings *keyBindingsManager = KeyBindings::instance();
    QList<KeyBinding> keyBindings = keyBindingsManager->keyBindings();
    Q_FOREACH (const KeyBinding &keyBinding, keyBindings) {
        if (keyBinding.matches(event->key(), event->modifiers())) {
            event->ignore();
            Q_EMIT keyBindingsManager->keyBindingPressed(keyBinding.name());
            return;
        }
    }

    QQuickWindow::keyPressEvent(event);
}

void OutputWindow::keyReleaseEvent(QKeyEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    // Key bindings
    KeyBindings *keyBindingsManager = KeyBindings::instance();
    QList<KeyBinding> keyBindings = keyBindingsManager->keyBindings();
    Q_FOREACH (const KeyBinding &keyBinding, keyBindings) {
        if (keyBinding.matches(event->key(), event->modifiers())) {
            event->ignore();
            Q_EMIT keyBindingsManager->keyBindingReleased(keyBinding.name());
            return;
        }
    }

    QQuickWindow::keyReleaseEvent(event);
}

void OutputWindow::mousePressEvent(QMouseEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickWindow::mousePressEvent(event);
}

void OutputWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickWindow::mouseReleaseEvent(event);
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

    QQuickWindow::mouseMoveEvent(event);
}

void OutputWindow::wheelEvent(QWheelEvent *event)
{
    m_output->compositor()->setState(Compositor::Active);

    QQuickWindow::wheelEvent(event);
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

void OutputWindow::showOutput()
{
    FullScreenShellClient *fsh = m_output->compositor()->d_func()->fullscreenShell;

    // Disable decorations for fullscreen shell
    if (fsh)
        setFlags(flags() | Qt::BypassWindowManagerHint);

    // Show
    show();

    // Show output on fullscreen shell
    if (fsh) {
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Showing output on full screen shell for output"
                << m_output->name() << m_output->geometry();
        fsh->showOutput(m_output);
    }
}

void OutputWindow::hideOutput()
{
    // Hide output from fullscreen shell
    FullScreenShellClient *fsh = m_output->compositor()->d_func()->fullscreenShell;
    if (fsh) {
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Hiding output on full screen shell for output"
                << m_output->name() << m_output->geometry();
        fsh->hideOutput(m_output);
    }

    // Hide
    hide();
}

void OutputWindow::readContent()
{
    // Send frame callbacks for windows rendered on this output
    m_output->compositor()->sendFrameCallbacks(m_output->compositor()->surfaces());

    // Record a frame after rendering
    m_output->compositor()->d_ptr->recorderManager->recordFrame(this);
}

void OutputWindow::statusChanged(QQmlComponent::Status status)
{
    switch (status) {
    case QQmlComponent::Null:
        qCWarning(GREENISLAND_COMPOSITOR) << "No source set yet";
        break;
    case QQmlComponent::Loading:
        qCDebug(GREENISLAND_COMPOSITOR) << "Loading QML scene...";
        break;
    case QQmlComponent::Ready:
        qCDebug(GREENISLAND_COMPOSITOR) << "QML scene ready";
        break;
    default:
        break;
    }

    continueLoading();
}

void OutputWindow::continueLoading()
{
    if (m_component->isError()) {
        qCWarning(GREENISLAND_COMPOSITOR) << "One or more errors have occurred loading the scene:";
        Q_FOREACH (const QQmlError &error, m_component->errors())
            qCWarning(GREENISLAND_COMPOSITOR) << "*" << error.toString();
        return;
    }

    QQuickItem *rootItem = qobject_cast<QQuickItem *>(m_component->create(m_context));
    rootItem->setParentItem(contentItem());
    rootItem->setSize(size());

    qCDebug(GREENISLAND_COMPOSITOR) << "Scene load time:" << m_perfTimer.elapsed() << "ms";

    // Show window
    if (!isVisible()) {
        qCDebug(GREENISLAND_COMPOSITOR)
                << "Showing window for output"
                << m_output->name() << m_output->geometry();
        showOutput();
    }
}

}

#include "moc_outputwindow.cpp"
