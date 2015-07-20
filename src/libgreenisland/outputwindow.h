/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLAND_OUTPUTWINDOW_H
#define GREENISLAND_OUTPUTWINDOW_H

#include <QtCore/QElapsedTimer>
#include <QtQml/QQmlComponent>
#include <QtQuick/QQuickWindow>

#include <greenisland/greenisland_export.h>

class QQmlContext;

namespace GreenIsland {

class Output;

class GREENISLAND_EXPORT OutputWindow : public QQuickWindow
{
    Q_OBJECT
public:
    OutputWindow(Output *output);
    ~OutputWindow();

    Output *output() const;

    void loadScene();
    void unloadScene();

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

private:
    Output *m_output;
    QQmlComponent *m_component;
    QQmlContext *m_context;
    QElapsedTimer m_perfTimer;

    // Hot spots
    QElapsedTimer m_timer;
    quint64 m_hotSpotLastTime;
    quint64 m_hotSpotEntered;

    void handleMotion(quint64 time, const QPoint &pt);

    void showOutput();
    void hideOutput();

private Q_SLOTS:
    void readContent();
    void statusChanged(QQmlComponent::Status status);
    void continueLoading();
};

}

#endif // GREENISLAND_OUTPUTWINDOW_H
