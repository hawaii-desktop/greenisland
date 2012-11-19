/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
#include <QRect>

class PanelView;
class LauncherView;

class DesktopShell : public QObject
{
    Q_OBJECT
public:
    explicit DesktopShell();
    ~DesktopShell();

    PanelView *panelView() const {
        return m_panelView;
    }
    LauncherView *launcherView() const {
        return m_launcherView;
    }

    void show();
    void hide();

private:
    PanelView *m_panelView;
    LauncherView *m_launcherView;

    static const struct wl_registry_listener registryListener;
};

#endif // DESKTOPSHELL_H
