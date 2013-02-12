/****************************************************************************
 * This file is part of Desktop Shell.
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
#include <QSocketNotifier>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

#include "cmakedirs.h"
#include "desktopshell.h"
#include "waylandintegration.h"
#include "launcher.h"
#include "background.h"

Q_GLOBAL_STATIC(DesktopShell, desktopShell)

DesktopShell::DesktopShell()
    : QObject()
{
    // Set path so that programs will be found
    QByteArray path = qgetenv("PATH");
    if (!path.isEmpty())
        path += ":";
    path += INSTALL_BINDIR;
    setenv("PATH", qPrintable(path), 1);

    // Platform native interface
    QPlatformNativeInterface *native =
            QGuiApplication::platformNativeInterface();
    Q_ASSERT(native);

    // Get Wayland display
    m_display = static_cast<struct wl_display *>(
                native->nativeResourceForIntegration("display"));
    Q_ASSERT(m_display);

    // Display file descriptor
    m_fd = wl_display_get_fd(m_display);
    Q_ASSERT(m_fd > -1);

    // Wayland registry
    m_registry = wl_display_get_registry(m_display);
    Q_ASSERT(m_registry);

    // Receive Wayland events
    QSocketNotifier *readNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(readNotifier, SIGNAL(activated(int)), this, SLOT(readEvents()));

    // Dispatch Wayland events
    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(awake()), this, SLOT(flushRequests()));

    // Wayland integration
    WaylandIntegration *integration = WaylandIntegration::createInstance(this);
    wl_registry_add_listener(m_registry, &WaylandIntegration::registryListener,
                             WaylandIntegration::instance());

    // Wait until QPA plugin detected all outputs and we registered the
    // desktop shell listener
    QElapsedTimer timeout;
    timeout.start();
    do {
        QGuiApplication::processEvents();
    } while (integration->protocolSync < 2 && timeout.elapsed() < 1000);

    // If we couldn't
    if (integration->protocolSync < 2)
        qFatal("Lost synchronization with compositor");

    qDebug() << "Number of screens detected:" << QGuiApplication::screens().size();

    foreach (QScreen *screen, QGuiApplication::screens()) {
        Output *output = new Output();

        qDebug() << "Creating shell surfaces on" << screen->name();

        m_outputs.append(output);

        // Get native wl_output for the current screen
        output->screen = screen;
        output->output = static_cast<struct wl_output *>(
                    native->nativeResourceForScreen("output", output->screen));

        // Set a wallpaper for each screen
        output->background = new Background(screen, this);
        output->backgroundSurface = static_cast<struct wl_surface *>(
                    native->nativeResourceForWindow("surface",
                                                    output->background->window()));
        wl_surface_set_user_data(output->backgroundSurface, output->background);
        desktop_shell_set_background(integration->shell, output->output,
                                     output->backgroundSurface);

#if 0
        // Create a launcher window for each output
        output->launcher = new Launcher(screen, this);
        output->launcherSurface = static_cast<struct wl_surface *>(
                    native->nativeResourceForWindow("surface", output->launcher));
        wl_surface_set_user_data(output->launcherSurface, output->launcher);
        output->launcher->setGeometry(QRect(0, 0, 1, 1));
        desktop_shell_set_panel(integration->shell, output->output,
                                output->launcherSurface);
#endif
    }
}

DesktopShell::~DesktopShell()
{
    foreach (Output *output, m_outputs) {
        m_outputs.removeOne(output);
        delete output;
    }
}

DesktopShell *DesktopShell::instance()
{
    return desktopShell();
}

void DesktopShell::readEvents()
{
    wl_display_dispatch(m_display);
}

void DesktopShell::flushRequests()
{
    wl_display_dispatch_pending(m_display);
    wl_display_flush(m_display);
}

#include "moc_desktopshell.cpp"
