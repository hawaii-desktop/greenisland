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

#include <QtCompositor/QWaylandSurfaceItem>
#include <QtCompositor/QWaylandInputDevice>

#include "clientwindow.h"
#include "compositor.h"

namespace GreenIsland {

ClientWindow::ClientWindow(QObject *parent)
    : QObject(parent)
    , m_mapped(false)
    , m_active(false)
    , m_minimized(false)
    , m_maximized(false)
    , m_fullScreen(false)
    , m_surface(nullptr)
{
}

QuickSurface *ClientWindow::surface() const
{
    return m_surface;
}

void ClientWindow::setSurface(QuickSurface *surface)
{
    if (m_surface == surface)
        return;

    m_surface = surface;
    Q_EMIT surfaceChanged();
    if (!m_surface)
        return;

    connect(m_surface, SIGNAL(titleChanged()), this, SIGNAL(titleChanged()));
    connect(m_surface, SIGNAL(classNameChanged()), this, SIGNAL(appIdChanged()));
    connect(m_surface, &QWaylandSurface::visibilityChanged, [=]() {
        if (!m_mapped)
            return;

        switch (m_surface->visibility()) {
        case QWindow::Minimized:
            m_minimized = true;
            break;
        case QWindow::Maximized:
            m_maximized = true;
            break;
        case QWindow::FullScreen:
            m_fullScreen = true;
            break;
        default:
            break;
        }
    });
    QObject::connect(m_surface, &QWaylandSurface::mapped, [=]() {
        if (m_mapped)
            return;

        m_mapped = true;

        Compositor *compositor = static_cast<Compositor *>(m_surface->compositor());
        QWaylandSurfaceItem *view = compositor->firstViewOf(m_surface);

        if (view) {
            QObject::connect(view, &QWaylandSurfaceItem::focusChanged, [=](bool focus) {
                m_active = focus;
            });
        }
    });
    QObject::connect(m_surface, &QWaylandSurface::unmapped, [=]() {
        m_mapped = false;
    });
}

QString ClientWindow::title() const
{
    return m_surface->title();
}

QString ClientWindow::appId() const
{
    return m_surface->className();
}

bool ClientWindow::isMapped() const
{
    return m_mapped;
}

bool ClientWindow::isActive() const
{
    return m_active;
}

void ClientWindow::activate()
{
    if (m_surface) {
        Compositor *compositor = static_cast<Compositor *>(m_surface->compositor());
        QWaylandSurfaceItem *view = compositor->firstViewOf(m_surface);
        if (view)
            view->takeFocus();
        m_active = true;
        Q_EMIT activeChanged();
    }
}

void ClientWindow::deactivate()
{
    if (m_surface) {
        Compositor *compositor = static_cast<Compositor *>(m_surface->compositor());
        QWaylandSurfaceItem *view = compositor->firstViewOf(m_surface);
        if (view) {
            view->setFocus(false);
            m_surface->compositor()->defaultInputDevice()->setKeyboardFocus(0);
        }
        m_active = true;
        Q_EMIT activeChanged();
    }
}

bool ClientWindow::isMinimized() const
{
    return m_minimized;
}

void ClientWindow::minimize()
{
    if (m_surface && !m_minimized) {
        m_surface->setVisibility(QWindow::Minimized);
        m_minimized = true;
        Q_EMIT minimizedChanged();
    }
}

void ClientWindow::unminimize()
{
    if (m_surface && m_minimized) {
        m_surface->setVisibility(QWindow::AutomaticVisibility);
        m_minimized = false;
        Q_EMIT minimizedChanged();
    }
}

bool ClientWindow::isMaximized() const
{
    return m_maximized;
}

void ClientWindow::maximize()
{
    if (m_surface && !m_maximized) {
        m_surface->setVisibility(QWindow::Maximized);
        m_maximized = true;
        Q_EMIT maximizedChanged();
    }
}

void ClientWindow::unmaximize()
{
    if (m_surface && m_maximized) {
        m_surface->setVisibility(QWindow::Windowed);
        m_maximized = false;
        Q_EMIT maximizedChanged();
    }
}

bool ClientWindow::isFullScreen() const
{
    return m_fullScreen;
}

void ClientWindow::setFullScreen(bool fs)
{
    if (m_surface && m_fullScreen != fs) {
        m_surface->setVisibility(fs ? QWindow::FullScreen : QWindow::AutomaticVisibility);
        m_fullScreen = fs;
        Q_EMIT fullScreenChanged();
    }
}

} // namespace GreenIsland
