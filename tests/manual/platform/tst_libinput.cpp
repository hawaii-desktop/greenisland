/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <GreenIsland/Platform/LibInputHandler>
#include <GreenIsland/Platform/Logind>

using namespace GreenIsland::Platform;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Logind *logind = Logind::instance();
    QObject::connect(logind, &Logind::connectedChanged, [logind](bool connected) {
        // Taking control on an already running session seems problematic:
        // screen goes black and the desktop crashes, so this is commented out
        // until I figure out what to do. Of course without taking control,
        // libinput won't have permission on the devices - unless the user is
        // in the input group.
        /*
        if (connected)
            logind->takeControl();
        */
        Q_UNUSED(connected);
    });

    LibInputHandler *handler = new LibInputHandler(&app);
    QObject::connect(handler, &LibInputHandler::keyPressed,
                     [](const LibInputKeyEvent &e) {
        qDebug() << "Key pressed:" << e.key;
    });
    QObject::connect(handler, &LibInputHandler::keyPressed,
                     [](const LibInputKeyEvent &e) {
        qDebug() << "Key released:" << e.key;
        if (e.key == Qt::Key_Q)
            QCoreApplication::instance()->quit();
    });
    QObject::connect(handler, &LibInputHandler::mousePressed,
                     [](const LibInputMouseEvent &e) {
        qDebug() << "Mouse pressed:" << e.buttons;
    });
    QObject::connect(handler, &LibInputHandler::mousePressed,
                     [](const LibInputMouseEvent &e) {
        qDebug() << "Mouse released:" << e.buttons;
    });
    QObject::connect(handler, &LibInputHandler::mouseMoved,
                     [](const LibInputMouseEvent &e) {
        qDebug() << "Mouse moved:" << e.pos;
    });
    QObject::connect(handler, &LibInputHandler::mouseWheel,
                     [](const LibInputMouseEvent &e) {
        qDebug() << "Mouse wheel:" << e.wheelDelta << e.wheelOrientation;
    });
    QObject::connect(handler, &LibInputHandler::touchEvent,
                     [](const LibInputTouchEvent &e) {
        qDebug() << "Touch" << e.touchPoints;
    });
    QObject::connect(handler, &LibInputHandler::touchCancel,
                     [](const LibInputTouchEvent &e) {
        qDebug() << "Touch cancel:" << e.touchPoints;
    });

    return app.exec();
}
