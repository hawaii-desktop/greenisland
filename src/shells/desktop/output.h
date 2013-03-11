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

#ifndef OUTPUT_H
#define OUTPUT_H

#include <QObject>

#include <wayland-client.h>

class QPlatformNativeInterface;
class QScreen;

class Background;
class Panel;
class Launcher;

class Output : public QObject
{
    Q_OBJECT
public:
    explicit Output(QScreen *screen);

    QScreen *screen() const {
        return m_screen;
    }

    struct wl_output *output() const {
        return m_output;
    }

    Background *background() const {
        return m_background;
    }

    struct wl_surface *backgroundSurface() const {
        return m_backgroundSurface;
    }

    void setBackground(Background *background);

    Panel *panel() const {
        return m_panel;
    }

    struct wl_surface *panelSurface() const {
        return m_panelSurface;
    }

    void setPanel(Panel *panel);

    Launcher *launcher() const {
        return m_launcher;
    }

    struct wl_surface *launcherSurface() const {
        return m_launcherSurface;
    }

    void setLauncher(Launcher *launcher);

public Q_SLOTS:
    void sendPanelGeometry();
    void sendLauncherGeometry();

private:
    QPlatformNativeInterface *m_native;

    QScreen *m_screen;
    struct wl_output *m_output;

    Background *m_background;
    struct wl_surface *m_backgroundSurface;

    Panel *m_panel;
    struct wl_surface *m_panelSurface;

    Launcher *m_launcher;
    struct wl_surface *m_launcherSurface;

private Q_SLOTS:
    void panelGeometryChanged(const QRect &geometry);
    void launcherGeometryChanged(const QRect &geometry);
};

#endif // OUTPUT_H
