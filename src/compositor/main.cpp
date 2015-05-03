/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QLoggingCategory>
#include <QtCore/QCommandLineParser>
#include <QtGui/QGuiApplication>

#include <greenisland/homeapplication.h>

#include "config.h"

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    // Application
    QGuiApplication app(argc, argv);
    app.setApplicationName("Green Island");
    app.setApplicationVersion(GREENISLAND_VERSION_STRING);
    app.setQuitOnLastWindowClosed(false);

    // Home application
    GreenIsland::HomeApplication homeApp;

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("QtQuick Wayland compositor"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Wayland socket
    QCommandLineOption socketOption(QStringList() << QStringLiteral("s") << QStringLiteral("socket"),
                                    TR("Wayland socket"), TR("name"));
    parser.addOption(socketOption);

    // Synthesize touch for unhandled mouse events
    QCommandLineOption synthesizeOption(QStringLiteral("synthesize-touch"),
                                        TR("Synthesize touch for unhandled mouse events"));
    parser.addOption(synthesizeOption);

    // Idle time
    QCommandLineOption idleTimeOption(QStringList() << QStringLiteral("i") << QStringLiteral("idle-time"),
                                      TR("Idle time in seconds (at least 5 seconds)"), TR("secs"));
    idleTimeOption.setDefaultValue("300");
    parser.addOption(idleTimeOption);

    // Login manager notifications
    QCommandLineOption notifyOption(QStringLiteral("notify"),
                                    TR("Notify login manager about startup"));
    parser.addOption(notifyOption);

    // Fake screen configuration
    QCommandLineOption fakeScreenOption(QStringLiteral("fake-screen"),
                                        TR("Use fake screen configuration"),
                                        TR("filename"));
    parser.addOption(fakeScreenOption);

    // Compositor package
    QCommandLineOption shellOption(QStringLiteral("shell"),
                                   TR("Force loading the given shell"),
                                   TR("shell"));
    parser.addOption(shellOption);

    // Parse command line
    parser.process(app);

    // Home application parameters
    homeApp.setSocket(parser.value(socketOption));
    homeApp.setNotifyLoginManager(parser.isSet(notifyOption));
    homeApp.setFakeScreenData(parser.value(fakeScreenOption));

    // Idle timer
    int idleInterval = parser.value(idleTimeOption).toInt();
    if (idleInterval >= 5)
        homeApp.setIdleTime(idleInterval * 1000);

    // Create the compositor and run
    if (!homeApp.run(parser.value(shellOption)))
        return 1;

    return app.exec();
}
