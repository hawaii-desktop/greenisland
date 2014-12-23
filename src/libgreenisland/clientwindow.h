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

#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <greenisland/quicksurface.h>

namespace GreenIsland {

class QuickSurface;

class GREENISLAND_EXPORT ClientWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool minimized READ isMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
public:
    ClientWindow(QObject *parent = 0);

    QuickSurface *surface() const;
    void setSurface(QuickSurface *surface);

    QString title() const;
    QString appId() const;

    bool isMapped() const;

    bool isActive() const;
    void activate();
    void deactivate();

    bool isMinimized() const;
    void minimize();
    void unminimize();

    bool isMaximized() const;
    void maximize();
    void unmaximize();

    bool isFullScreen() const;
    void setFullScreen(bool fs);

Q_SIGNALS:
    void surfaceChanged();
    void titleChanged();
    void appIdChanged();
    void activeChanged();
    void minimizedChanged();
    void maximizedChanged();
    void fullScreenChanged();

private:
    bool m_mapped;
    bool m_active;
    bool m_minimized;
    bool m_maximized;
    bool m_fullScreen;
    QuickSurface *m_surface;
};

} // namespace GreenIsland

#endif // CLIENTWINDOW_H
