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
#include <QtCore/QTimer>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandOutput>
#include <QtCompositor/private/qwloutput_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "applicationmanager_p.h"
#include "clientwindow.h"
#include "compositor.h"
#include "compositor_p.h"
#include "output.h"
#include "windowview.h"

#include "protocols/gtk-shell/gtksurface.h"

namespace GreenIsland {

ClientWindow::ClientWindow(QWaylandSurface *surface, QObject *parent)
    : QObject(parent)
    , m_appId(surface->className())
    , m_pos(QPointF(surface->mainOutput()->availableGeometry().topLeft()))
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

    // Set size from surface
    m_size = QSizeF(surface->size());
    m_internalGeometry = QRectF(QPointF(0, 0), m_size);

    // Create a view for the surface output, views for other outputs
    // will be created on demand
    viewForOutput(static_cast<Output *>(surface->mainOutput()));

    // Connect to surface signals
    connect(m_surface, &QWaylandSurface::mapped,
            this, &ClientWindow::surfaceMapped);
    connect(m_surface, &QWaylandSurface::unmapped,
            this, &ClientWindow::surfaceUnmapped);
    connect(m_surface, &QWaylandSurface::titleChanged,
            this, &ClientWindow::titleChanged);
    connect(m_surface, &QWaylandSurface::classNameChanged,
            this, &ClientWindow::surfaceAppIdChanged);
    connect(m_surface, &QWaylandSurface::sizeChanged,
            this, &ClientWindow::surfaceSizeChanged);
    connect(m_surface, &QWaylandSurface::windowTypeChanged,
            this, &ClientWindow::setType);
    connect(m_surface, &QWaylandSurface::parentChanged,
            this, &ClientWindow::parentSurfaceChanged);

    // Determine the app_id here because some applications will
    // never send set_app_id to the shell surface (i.e. weston-terminal)
    determineAppId();

    // Add window to the list
    m_compositor->d_func()->addWindow(this);
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
    return m_parentWindow.data();
}

QWaylandOutput *ClientWindow::output() const
{
    // Find the output that contains the biggest part of this window,
    // that is the main output and it will be used by effects such as
    // present windows to present only windows for the output it is
    // running on (effects run once for each output)
    int maxArea = 0, area = 0;
    QWaylandOutput *main = m_surface->mainOutput();

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
        connect(view, &WindowView::mousePressed, [this] {
            activate();
        });
    }

    return view;
}

WindowView *ClientWindow::parentViewForOutput(Output *output)
{
    // Spare the loop if the surface doesn't have a parent
    if (!m_surface->transientParent())
        return Q_NULLPTR;

    // Find the transient parent window
    Q_FOREACH (ClientWindow *parentWindow, m_compositor->d_func()->clientWindowsList) {
        if (parentWindow->surface() == m_surface->transientParent())
            return parentWindow->viewForOutput(output);
    }

    return Q_NULLPTR;
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

    // Set main output and send enter/leave events
    Output *oldOutput = static_cast<Output *>(m_surface->mainOutput());
    Output *newOutput = static_cast<Output *>(output());
    if (newOutput != oldOutput) {
        m_surface->setMainOutput(newOutput);
        m_surface->handle()->removeFromOutput(oldOutput->handle());
        m_surface->handle()->addToOutput(newOutput->handle());
    }

    // ClientWindow position is in compositor space, but the view
    // position is expressed in output space; QML window representation
    // will be moved according to view coordinates using
    // property binding
    Q_FOREACH (Output *o, m_views.keys())
        m_views[o]->setLocalPosition(o->mapToOutput(m_pos));
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
    Q_EMIT ApplicationManager::instance()->applicationFocused(m_appId);
    Q_FOREACH (const QString &appId, ApplicationManager::instance()->d_func()->apps) {
        if (appId == m_appId)
            continue;
        Q_EMIT ApplicationManager::instance()->applicationUnfocused(appId);
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

void ClientWindow::terminateProcess(quint32 timeout)
{
    if (m_surface && m_surface->client()) {
        pid_t pid = m_surface->client()->processId();

        if (pid == QCoreApplication::applicationPid())
            return;

        ::kill(pid, SIGTERM);
        QTimer::singleShot(timeout, [pid] {
            ::kill(pid, SIGKILL);
        });
    }
}

void ClientWindow::registerWindow()
{
    // Register this window
    if (!m_appId.isEmpty())
        ApplicationManager::instance()->d_func()->registerSurface(m_surface, m_appId);
    m_compositor->d_func()->mapWindow(this);
}

void ClientWindow::unregisterWindow(bool destruction)
{
    // Unregister this window
    if (destruction)
        m_compositor->d_func()->destroyWindow(this);
    else
        m_compositor->d_func()->unmapWindow(this);
    if (!m_appId.isEmpty())
        ApplicationManager::instance()->d_func()->unregisterSurface(m_surface, m_appId);
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

    // Honor position for windows that start maximized or full screen
    if (isFullScreen() || isMaximized())
        return;

    // Set initial position
    switch (m_surface->windowType()) {
    case QWaylandSurface::Popup:
        // Move popups relative to parent window
        if (parentWindow()) {
#if 0
            m_pos.setX(parentWindow()->position().x() + m_surface->transientOffset().x());
            m_pos.setY(parentWindow()->position().y() + m_surface->transientOffset().y());
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
        if (parentWindow()) {
#if 0
            m_pos.setX(parentWindow()->position().x() +
                       ((parentWindow()->size().width() - m_size.width()) / 2));
            m_pos.setY(parentWindow()->position().y() +
                       ((parentWindow()->size().height() - m_size.height()) / 2));
#else
            m_pos.setX((m_surface->transientParent()->size().width() - m_size.width()) / 2);
            m_pos.setY((m_surface->transientParent()->size().height() - m_size.height()) / 2);
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

void ClientWindow::determineAppId()
{
    QString appId, iconName;

    // Try Gtk+ application identifier
    Q_FOREACH (QWaylandSurfaceInterface *interface, m_surface->interfaces()) {
        GtkSurface *gtkSurface = dynamic_cast<GtkSurface *>(interface);
        if (gtkSurface) {
            // Check if we have a desktop entry with that name
            const QString tmpAppId = gtkSurface->applicationId();
            if (!findDesktopFile(tmpAppId).isEmpty()) {
                appId = tmpAppId;
                break;
            }
        }
    }

    // Use the app_id from the surface
    if (appId.isEmpty()) {
        const QString tmpAppId = m_surface->className()
                .replace(QStringLiteral(".desktop"), QString())
                .toLower();
        if (findDesktopFile(tmpAppId).isEmpty()) {
            // Try with vendor prefixes (this is not an exhaustive list)
            QStringList vendors = QStringList()
                    << QStringLiteral("org.hawaii.")
                    << QStringLiteral("org.kde.")
                    << QStringLiteral("org.gnome.");
            Q_FOREACH (QString vendor, vendors) {
                if (!findDesktopFile(vendor + tmpAppId).isEmpty()) {
                    appId = vendor + tmpAppId;
                    break;
                }
            }
        } else {
            appId = tmpAppId;
        }
    }

    // Use process name if appId is empty (some applications won't set it, like weston-terminal)
    if (appId.isEmpty()) {
        QFile file(QStringLiteral("/proc/%1/cmdline").arg(m_surface->client()->processId()));
        if (file.open(QIODevice::ReadOnly)) {
            QFileInfo fi(QString::fromUtf8(file.readAll().split(' ').at(0)));
            appId = fi.baseName();
            file.close();
        }
    }

    // Find desktop entry
    m_desktopEntry = findDesktopFile(appId);

    // Read icon name from the desktop entry
    if (iconName.isEmpty())
        iconName = readFromDesktopFile(QStringLiteral("Icon"),
                                       QStringLiteral("application-octet-stream")).toString();

    qDebug() << "Found desktop entry" << m_desktopEntry << "for" << appId;

    m_appId = appId;
    m_iconName = iconName;
}

QString ClientWindow::findDesktopFile(const QString &appId)
{
    return QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                  QStringLiteral("%1.desktop").arg(appId));
}

QVariant ClientWindow::readFromDesktopFile(const QString &key, const QVariant &defaultValue) const
{
    QSettings entry(m_desktopEntry, QSettings::IniFormat);
    entry.setIniCodec("UTF-8");
    entry.beginGroup(QStringLiteral("Desktop Entry"));
    return entry.value(key, defaultValue);
}

void ClientWindow::surfaceMapped()
{
    initialSetup();
    registerWindow();
}

void ClientWindow::surfaceUnmapped()
{
    // When a Qt client is closed, it will send unmapped() but the
    // surface size will be invalid: in this case we should emit
    // a window destruction signal instead of unmapped
    unregisterWindow(m_surface && m_surface->size().isValid() ? false : true);
}

void ClientWindow::surfaceAppIdChanged()
{
    determineAppId();
    Q_EMIT appIdChanged();
    Q_EMIT iconNameChanged();
}

void ClientWindow::surfaceSizeChanged()
{
    setSize(QSizeF(m_surface->size()));
}

void ClientWindow::setType(QWaylandSurface::WindowType windowType)
{
    // Save type
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

void ClientWindow::parentSurfaceChanged(QWaylandSurface *newParent,
                                        QWaylandSurface *oldParent)
{
    Q_UNUSED(oldParent)

    // Find the transient parent window
    Q_FOREACH (ClientWindow *parentWindow, m_compositor->d_func()->clientWindowsList) {
        if (parentWindow->surface() == newParent) {
            m_parentWindow = parentWindow;
            break;
        }
    }
}

} // namespace GreenIsland
