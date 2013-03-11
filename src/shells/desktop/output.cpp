/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (C) 2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QGuiApplication>
#include <QScreen>
#include <QQuickWindow>

#include <qpa/qplatformnativeinterface.h>

#include "output.h"
#include "background.h"
#include "panel.h"
#include "launcher.h"
#include "waylandintegration.h"

Output::Output(QScreen *screen)
    : QObject()
    , m_screen(screen)
    , m_background(0)
    , m_backgroundSurface(0)
    , m_panel(0)
    , m_panelSurface(0)
    , m_launcher(0)
    , m_launcherSurface(0)
{
    // Native platform interface
    m_native = QGuiApplication::platformNativeInterface();

    // Get native wl_output for the current screen
    m_output = static_cast<struct wl_output *>(
                m_native->nativeResourceForScreen("output", screen));
}

void Output::setBackground(Background *background)
{
    if (!background)
        return;

    m_background = background;
    m_backgroundSurface = static_cast<struct wl_surface *>(
                m_native->nativeResourceForWindow("surface",
                                                  background->window()));
}

void Output::setPanel(Panel *panel)
{
    if (!panel)
        return;

    m_panel = panel;
    m_panelSurface = static_cast<struct wl_surface *>(
                m_native->nativeResourceForWindow("surface",
                                                  panel->window()));
}

void Output::setLauncher(Launcher *launcher)
{
    if (!launcher)
        return;

    m_launcher = launcher;
    m_launcherSurface = static_cast<struct wl_surface *>(
                m_native->nativeResourceForWindow("surface",
                                                  launcher->window()));
}

void Output::sendPanelGeometry()
{
    if (m_panel)
        panelGeometryChanged(m_panel->geometry());
}

void Output::sendLauncherGeometry()
{
    if (m_launcher)
        launcherGeometryChanged(m_launcher->geometry());
}

void Output::panelGeometryChanged(const QRect &geometry)
{
    WaylandIntegration *integration = WaylandIntegration::instance();
    desktop_shell_set_panel_geometry(integration->shell, m_output,
                                     m_panelSurface, geometry.x(), geometry.y(),
                                     geometry.width(), geometry.height());
}

void Output::launcherGeometryChanged(const QRect &geometry)
{
    WaylandIntegration *integration = WaylandIntegration::instance();
    desktop_shell_set_launcher_geometry(integration->shell, m_output,
                                        m_launcherSurface, geometry.x(), geometry.y(),
                                        geometry.width(), geometry.height());
}

#include "moc_output.cpp"
