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

#ifndef DESKTOPSHELL_H
#define DESKTOPSHELL_H

#include <QObject>

class QScreen;

class Background;
class Panel;
class Launcher;

class Output
{
public:
    Output()
        : screen(0)
        , output(0)
        , background(0)
        , backgroundSurface(0)
        , panel(0)
        , panelSurface(0)
        , launcher(0)
        , launcherSurface(0)
    {
    }

    QScreen *screen;
    struct wl_output *output;

    Background *background;
    struct wl_surface *backgroundSurface;

    Panel *panel;
    struct wl_surface *panelSurface;

    Launcher *launcher;
    struct wl_surface *launcherSurface;
};

class DesktopShell : public QObject
{
    Q_OBJECT
public:
    explicit DesktopShell();
    ~DesktopShell();

    static DesktopShell *instance();

    QList<Output *> outputs() const {
        return m_outputs;
    }

    void addOutput(Output *output) {
        m_outputs.append(output);
    }

public Q_SLOTS:
    void create();

private:
    struct wl_display *m_display;
    int m_fd;
    struct wl_registry *m_registry;
    QList<Output *> m_outputs;
};

#endif // DESKTOPSHELL_H
