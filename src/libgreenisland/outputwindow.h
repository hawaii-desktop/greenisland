/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLAND_OUTPUTWINDOW_H
#define GREENISLAND_OUTPUTWINDOW_H

#include <QtCore/QElapsedTimer>
#include <QtQuick/QQuickView>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class Compositor;
class Output;

class GREENISLAND_EXPORT OutputWindow : public QQuickView
{
    Q_OBJECT
public:
    explicit OutputWindow(Compositor *compositor);

    Compositor *compositor() const;

    Output *output() const;
    void setOutput(Output *output);

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

private:
    Compositor *m_compositor;
    Output *m_output;

    // Hot spots
    QElapsedTimer m_timer;
    quint64 m_hotSpotLastTime;
    quint64 m_hotSpotEntered;

    void handleMotion(quint64 time, const QPoint &pt);

private Q_SLOTS:
    void printInfo();
    void readContent();
};

}

#endif // GREENISLAND_OUTPUTWINDOW_H
