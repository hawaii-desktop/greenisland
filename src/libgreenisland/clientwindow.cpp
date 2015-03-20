/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2013-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandOutput>

#include "applicationmanager_p.h"
#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "output.h"
#include "windowview.h"

namespace GreenIsland {

ClientWindow::ClientWindow(QWaylandSurface *surface, QObject *parent)
    : QObject(parent)
    , m_appId(surface->className())
    , m_pos(QPointF(surface->output()->availableGeometry().topLeft()))
    , m_internalGeometryChanged(false)
    , m_active(false)
    , m_minimized(false)
    , m_maximized(false)
    , m_fullScreen(false)
    , m_initialSetup(false)
    , m_compositor(static_cast<Compositor *>(surface->compositor()))
    , m_surface(surface)
    , m_parentWindow(Q_NULLPTR)
{
    qRegisterMetaType<ClientWindow *>("ClientWindow*");

    // Identifier
    static uint id = 0;
    m_id = id++;

    // Set window type
    setType(m_surface->windowType());

    // Use process name if appId is empty (some applications won't set it, like weston-terminal)
    if (m_appId.isEmpty()) {
        QFile file(QStringLiteral("/proc/%1/cmdline").arg(surface->client()->processId()));
        if (file.open(QIODevice::ReadOnly)) {
            QFileInfo fi(QString::fromUtf8(file.readAll().split(' ').at(0)));
            m_appId = fi.baseName();
            m_iconName = QStringLiteral("application-octet-stream");
            file.close();
        }
    }

    // Set size from surface
    m_size = QSizeF(surface->size());
    m_internalGeometry = QRectF(QPointF(0, 0), m_size);

    // Create a view for the surface output, views for other outputs
    // will be created on demand
    viewForOutput(static_cast<Output *>(surface->output()));

    // Connect to surface signals
    connect(surface, &QWaylandSurface::mapped, [=] {
        initialSetup();
        registerWindow();
    });
    connect(surface, &QWaylandSurface::unmapped, [=] {
        unregisterWindow(false);
    });
    connect(surface, SIGNAL(titleChanged()), this, SIGNAL(titleChanged()));
    connect(surface, &QWaylandSurface::classNameChanged, [=] {
        if (!m_surface->className().isEmpty()) {
            m_appId = m_surface->className().replace(QStringLiteral(".desktop"), QString());
            m_iconName = readFromDesktopFile(m_appId, QStringLiteral("Icon"),
                                             QStringLiteral("application-octet-stream")).toString();
            Q_EMIT appIdChanged();
            Q_EMIT iconNameChanged();
        }
    });
    connect(surface, &QWaylandSurface::windowTypeChanged,
            this, &ClientWindow::setType);
    connect(surface, &QWaylandSurface::sizeChanged, [=] {
        setSize(QSizeF(m_surface->size()));
    });
}

ClientWindow::~ClientWindow()
{
    unregisterWindow(true);
}

uint ClientWindow::id() const
{
    return m_id;
}

QWaylandSurface *ClientWindow::surface() const
{
    return m_surface;
}

ClientWindow::Type ClientWindow::type() const
{
    return m_type;
}

QString ClientWindow::title() const
{
    return m_surface->title();
}

QString ClientWindow::appId() const
{
    return m_appId;
}

QString ClientWindow::iconName() const
{
    return m_iconName;
}

ClientWindow *ClientWindow::parentWindow() const
{
    return m_parentWindow;
}

QWaylandOutput *ClientWindow::output() const
{
    // Find the output that contains the biggest part of this window,
    // that is the main output and it will be used by effects such as
    // present windows to present only windows for the output it is
    // running on (effects run once for each output)
    int maxArea = 0, area = 0;
    QWaylandOutput *main = m_surface->output();

    Q_FOREACH (QWaylandOutput *output, m_surface->compositor()->outputs()) {
        QRectF intersection = QRectF(output->geometry()).intersected(geometry());

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

WindowView *ClientWindow::viewForOutput(Output *output)
{
    WindowView *view = m_views.value(output, Q_NULLPTR);
    if (!view) {
        view = static_cast<WindowView *>(m_surface->compositor()->createView(m_surface));
        view->setLocalPosition(output->mapToOutput(m_pos));
        m_views[output] = view;

        // Activate this window when the mouse is pressed
        connect(view, &WindowView::mousePressed, [=] {
            activate();
        });
    }

    return view;
}

qreal ClientWindow::x() const
{
    return m_pos.x();
}

void ClientWindow::setX(qreal value)
{
    setPosition(QPointF(value, m_pos.y()));
}

qreal ClientWindow::y() const
{
    return m_pos.y();
}

void ClientWindow::setY(qreal value)
{
    setPosition(QPointF(m_pos.x(), value));
}

QPointF ClientWindow::position() const
{
    return m_pos;
}

void ClientWindow::setPosition(const QPointF &pos)
{
    if (m_pos == pos)
        return;

    m_pos = pos;
    Q_EMIT positionChanged();
    Q_EMIT geometryChanged();

    // ClientWindow position is in compositor space, but the view
    // position is expressed in output space; QML window representation
    // will be moved according to view coordinates using
    // property binding
    Q_FOREACH (Output *o, m_views.keys()) {
        m_views[o]->setLocalPosition(o->mapToOutput(m_pos));
    }
}

QSizeF ClientWindow::size() const
{
    return m_size;
}

void ClientWindow::setSize(const QSizeF &size)
{
    if (m_size == size)
        return;

    m_size = size;
    Q_EMIT sizeChanged();
    Q_EMIT geometryChanged();

    if (!m_internalGeometryChanged && m_internalGeometry.size() != size) {
        m_internalGeometry.setSize(size);
        Q_EMIT internalGeometryChanged();
    }
}

QRectF ClientWindow::geometry() const
{
    return QRectF(m_pos, m_size);
}

QRectF ClientWindow::internalGeometry() const
{
    return m_internalGeometry;
}

void ClientWindow::setInternalGeometry(const QRectF &geometry)
{
    if (m_internalGeometry == geometry)
        return;

    m_internalGeometry = geometry;
    m_internalGeometryChanged = true;
    Q_EMIT internalGeometryChanged();
}

bool ClientWindow::isActive() const
{
    return m_active;
}

void ClientWindow::activate()
{
    if (m_active)
        return;

    m_active = true;
    Q_EMIT activeChanged();

    // Deactivate other windows
    Q_FOREACH (ClientWindow *w, m_compositor->d_func()->clientWindowsList) {
        if (w == this)
            continue;
        w->deactivate();
    }

    // Application has focus
    Q_EMIT m_compositor->applicationManager()->applicationFocused(m_appId);
    Q_FOREACH (const QString &appId, m_compositor->applicationManager()->d_func()->apps) {
        if (appId == m_appId)
            continue;
        Q_EMIT m_compositor->applicationManager()->applicationUnfocused(appId);
    }
}

void ClientWindow::deactivate()
{
    if (!m_active)
        return;

    m_active = false;
    Q_EMIT activeChanged();
}

bool ClientWindow::isMinimized() const
{
    return m_minimized;
}

void ClientWindow::minimize()
{
    if (m_minimized)
        return;

    m_minimized = true;
    Q_EMIT minimizedChanged();
}

void ClientWindow::unminimize()
{
    if (!m_minimized)
        return;

    m_minimized = false;
    Q_EMIT minimizedChanged();
}

bool ClientWindow::isMaximized() const
{
    return m_maximized;
}

void ClientWindow::maximize()
{
    if (m_maximized)
        return;

    maximizeForOutput(static_cast<Output *>(output()));

    m_maximized = true;
    Q_EMIT maximizedChanged();
}

void ClientWindow::unmaximize()
{
    if (!m_maximized)
        return;

    m_maximized = false;
    Q_EMIT maximizedChanged();
}

bool ClientWindow::isFullScreen() const
{
    return m_fullScreen;
}

void ClientWindow::setFullScreen(bool fs)
{
    if (m_fullScreen == fs)
        return;

    m_fullScreen = fs;
    Q_EMIT fullScreenChanged();
}

void ClientWindow::move()
{
    QWaylandSurfaceOp op(Move);
    m_surface->sendInterfaceOp(op);
}

void ClientWindow::stopMove()
{
    QWaylandSurfaceOp op(StopMove);
    m_surface->sendInterfaceOp(op);
}

void ClientWindow::registerWindow()
{
    // Register this window
    m_compositor->applicationManager()->d_func()->registerSurface(m_surface, m_appId);
    m_compositor->d_func()->mapWindow(this);
}

void ClientWindow::unregisterWindow(bool destruction)
{
    // Unregister this window
    if (destruction)
        m_compositor->d_func()->destroyWindow(this);
    else
        m_compositor->d_func()->unmapWindow(this);
    m_compositor->applicationManager()->d_func()->unregisterSurface(m_surface, m_appId);
}

QPointF ClientWindow::calculateInitialPosition() const
{
    // As a heuristic place the new window on the same output
    // as the pointer
    // TODO: Do something clever for touch too
    QPoint pos = QCursor::pos();

    // Find the target screen (the one where the coordinates are in)
    QRect geometry;
    bool targetScreenFound = false;
    for (QWaylandOutput *output: m_surface->compositor()->outputs()) {
        geometry = output->availableGeometry();
        if (geometry.contains(pos)) {
            targetScreenFound = true;
            break;
        }
    }

    // Fallback to the primary output
    if (!targetScreenFound)
        geometry = m_surface->compositor()->primaryOutput()->availableGeometry();

    // Valid range within output where the surface will still be onscreen.
    // If this is negative it means that the surface is bigger than
    // output in this case we fallback to 0,0 in available geometry space.
    int rangeX = geometry.size().width() - m_surface->size().width();
    int rangeY = geometry.size().height() - m_surface->size().height();

    int dx = 0, dy = 0;
    if (rangeX > 0)
        dx = (qrand() % rangeX) - geometry.x();
    if (rangeY > 0)
        dy = (qrand() % rangeY) - geometry.y();

    // Set surface position
    pos.setX(geometry.x() + dx);
    pos.setY(geometry.y() + dy);

    return pos;
}

void ClientWindow::initialSetup()
{
    if (m_initialSetup)
        return;

    if (!m_parentWindow) {
        // Find the transient parent window
        for (ClientWindow *parentWindow: m_compositor->d_func()->clientWindowsList) {
            if (parentWindow->surface() == m_surface->transientParent()) {
                m_parentWindow = parentWindow;
                break;
            }
        }
    }

    // Honor full screen positioning for top level windows that
    // start that way
    if (m_surface->windowType() == QWaylandSurface::Toplevel && isFullScreen())
        return;

    // Set initial position
    switch (m_surface->windowType()) {
    case QWaylandSurface::Popup:
        // Move popups relative to parent window
        if (m_parentWindow) {
#if 0
            m_pos.setX(m_parentWindow->position().x() + m_surface->transientOffset().x());
            m_pos.setY(m_parentWindow->position().y() + m_surface->transientOffset().y());
#else
            m_pos.setX(m_surface->transientOffset().x());
            m_pos.setY(m_surface->transientOffset().y());
#endif
        } else {
            m_pos.setX(m_surface->transientOffset().x());
            m_pos.setY(m_surface->transientOffset().y());
        }
        break;
    case QWaylandSurface::Transient:
        // Center transient windows
        if (m_parentWindow) {
#if 0
            m_pos.setX(m_parentWindow->position().x() +
                     ((m_parentWindow->size().width() - m_size.width()) / 2));
            m_pos.setY(m_parentWindow->position().y() +
                     ((m_parentWindow->size().height() - m_size.height()) / 2));
#else
            m_pos.setX((m_parentWindow->size().width() - m_size.width()) / 2);
            m_pos.setY((m_parentWindow->size().height() - m_size.height()) / 2);
#endif
        } else {
            m_pos.setX(0);
            m_pos.setY(0);
        }
        break;
    default:
        // Calculate initial position for top level windows
        m_pos = calculateInitialPosition();
        break;
    }

    Q_FOREACH (Output *o, m_views.keys()) {
        m_views[o]->setLocalPosition(o->mapToOutput(m_pos));
    }

    //m_initialSetup = true;
}

void ClientWindow::removeOutput(Output *output)
{
    m_views.take(output)->deleteLater();
}

void ClientWindow::maximizeForOutput(Output *output)
{
    setPosition(QPointF(output->availableGeometry().topLeft()));

    QWaylandSurfaceResizeOp op(output->availableGeometry().size());
    m_surface->sendInterfaceOp(op);
}

QVariant ClientWindow::readFromDesktopFile(const QString &baseName, const QString &key, const QVariant &defaultValue) const
{
    QString fileName = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                              baseName);
    QSettings entry(fileName, QSettings::IniFormat);
    entry.setIniCodec("UTF-8");
    entry.beginGroup(QStringLiteral("Desktop Entry"));
    return entry.value(key, defaultValue);
}

void ClientWindow::setType(QWaylandSurface::WindowType windowType)
{
    switch (windowType) {
    case QWaylandSurface::Transient:
        m_type = Transient;
        break;
    case QWaylandSurface::Popup:
        m_type = Popup;
        break;
    default:
        m_type = TopLevel;
        break;
    }

    Q_EMIT typeChanged();
}

} // namespace GreenIsland
