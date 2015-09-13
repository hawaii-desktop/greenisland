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

#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <QtDBus/QDBusServiceWatcher>
#include <QtGui/QGuiApplication>
#include <QtCompositor/private/qwlcompositor_p.h>

#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "fakescreenbackend.h"
#include "logging.h"
#include "nativescreenbackend.h"
#include "output.h"
#include "outputwindow.h"
#include "screenmanager.h"

namespace GreenIsland {

ScreenManager::ScreenManager(Compositor *compositor)
    : QObject()
    , m_compositor(compositor)
    , m_backend(Q_NULLPTR)
{
}

void ScreenManager::acquireConfiguration(const QString &fileName)
{
    if (m_backend) {
        qCWarning(GREENISLAND_COMPOSITOR) << "Cannot change backend at runtime!";
        return;
    }

    if (fileName.isEmpty())
        m_backend = new NativeScreenBackend(m_compositor, this);
    else
        m_backend = new FakeScreenBackend(m_compositor, this);

    connect(m_backend, &ScreenBackend::configurationAcquired,
            this, &ScreenManager::configurationAcquired);
    connect(m_backend, &ScreenBackend::outputAdded,
            this, &ScreenManager::outputAdded);
    connect(m_backend, &ScreenBackend::outputRemoved,
            this, &ScreenManager::outputRemoved);
    connect(m_backend, &ScreenBackend::primaryOutputChanged,
            this, &ScreenManager::primaryOutputChanged);

    if (!fileName.isEmpty())
        static_cast<FakeScreenBackend *>(m_backend)->loadConfiguration(fileName);

    m_backend->acquireConfiguration();
}

void ScreenManager::outputAdded(Output *output)
{
    // Debug
    qCDebug(GREENISLAND_COMPOSITOR)
            << "Added" << (output->isPrimary() ? "primary" : "") << "output"
            << output->name() << "with geometry" << output->geometry();

    // Set it as primary output
    if (output->isPrimary())
        m_compositor->setPrimaryOutput(output);

    // Emit signal for the compositor
    Q_EMIT m_compositor->outputAdded(output);

    // Load scene
    QMetaObject::invokeMethod(output, "loadScene", Qt::QueuedConnection);
}

void ScreenManager::outputRemoved(Output *output)
{
    // Find new primary output
    Output *primaryOutput = qobject_cast<Output *>(m_compositor->primaryOutput());
    if (!primaryOutput)
        return;

    // Geometry of the removed output
    QRectF removedGeometry(output->geometry());

    // Remove surface views and window representations of this output
    Q_FOREACH (ClientWindow *window, m_compositor->d_func()->clientWindowsList) {
        if (window->output() != output)
            continue;

        // Recalculate local coordinates
        qreal x = (window->x() * primaryOutput->geometry().width()) / removedGeometry.width();
        qreal y = (window->y() * primaryOutput->geometry().height()) / removedGeometry.height();
        window->setPosition(QPointF(x, y));

        // Set new global position
        window->setPosition(primaryOutput->mapToGlobal(QPointF(x, y)));

        // Ask the window to remove all views for the removed output
        window->removeOutput(output);
    }

    // Delete window and output
    output->window()->deleteLater();
    output->deleteLater();

    // Emit signal for the compositor
    Q_EMIT m_compositor->outputRemoved(output);

    // Debug
    qCDebug(GREENISLAND_COMPOSITOR) << "Removed output" << output->name() << output->geometry();
}

void ScreenManager::primaryOutputChanged(Output *output)
{
    m_compositor->setPrimaryOutput(output);
}

}

#include "moc_screenmanager.cpp"
