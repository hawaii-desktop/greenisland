/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2016 Pier Luigi Fiorini
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

#include <QtCore/QLoggingCategory>
#include <QtCore/QCommandLineParser>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>

#include <GreenIsland/Server/HomeApplication>

#include "config.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    // Application
    QGuiApplication app(argc, argv);
    app.setApplicationName("Green Island");
    app.setApplicationVersion(GREENISLAND_VERSION_STRING);
    app.setOrganizationName(QStringLiteral("Hawaii"));
    app.setOrganizationDomain(QStringLiteral("hawaiios.org"));
    app.setQuitOnLastWindowClosed(false);

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("QtQuick Wayland compositor"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Wayland socket
    QCommandLineOption socketOption(QStringLiteral("wayland-socket-name"),
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

#if HAVE_SYSTEMD
    // systemd notifications
    QCommandLineOption notifyOption(QStringLiteral("systemd"),
                                    TR("Notify systemd about startup"));
    parser.addOption(notifyOption);
#endif

    // Nested mode
    QCommandLineOption nestedOption(QStringList() << QStringLiteral("n") << QStringLiteral("nested"),
                                    TR("Nest into a compositor that supports _wl_fullscreen_shell"));
    parser.addOption(nestedOption);

    // Fake screen configuration
    QCommandLineOption fakeScreenOption(QStringLiteral("fake-screen"),
                                        TR("Use fake screen configuration"),
                                        TR("filename"));
    parser.addOption(fakeScreenOption);

    // Compositor package
    QCommandLineOption shellOption(QStringLiteral("shell"),
                                   TR("Load the given shell"),
                                   TR("shell"),
                                   QStringLiteral("org.hawaiios.greenisland"));
    parser.addOption(shellOption);

    // Shell main file
    QCommandLineOption qmlOption(QStringLiteral("qml"),
                                 TR("Load a shell main QML file, takes " \
                                    "precedence over the --shell argument."),
                                 TR("filename"));
    parser.addOption(qmlOption);

    // Parse command line
    parser.process(app);

    // Arguments
    bool nested = parser.isSet(nestedOption);
    QString socket = parser.value(socketOption);
#if HAVE_SYSTEMD
    bool notify = parser.isSet(notifyOption);
#else
    bool notify = false;
#endif
    QString fakeScreenData = parser.value(fakeScreenOption);
    int idleInterval = parser.value(idleTimeOption).toInt();
    if (idleInterval >= 5)
        idleInterval *= 1000;
    else
        idleInterval = -1;

    // Nested mode requires running from Wayland and a socket name
    // and fake screen data cannot be used
    if (nested) {
        if (!QGuiApplication::platformName().startsWith(QStringLiteral("greenisland"))) {
            qCritical("Nested mode only make sense when running on Wayland.\n"
                      "Please pass the \"-platform greenisland\" argument.");
#if HAVE_SYSTEMD
            if (notify)
                sd_notifyf(0, "STATUS=Nested mode requested, wrong QPA plugin");
#endif
            return 1;
        }

        if (socket.isEmpty()) {
            qCritical("Nested mode requires you to specify a socket name for Green Island.\n"
                      "Please specify it with the \"--wayland-socket-name\" argument.");
#if HAVE_SYSTEMD
            if (notify)
                sd_notifyf(0, "STATUS=Nested mode without a socket name specified");
#endif
            return 1;
        }

        if (!fakeScreenData.isEmpty()) {
            qCritical("Fake screen configuration cannot be used "
                      "when Green Island is nested");
#if HAVE_SYSTEMD
            if (notify)
                sd_notifyf(0, "STATUS=Fake screen configuration not allowed when nested");
#endif
            return 1;
        }
    }

    // Run
    GreenIsland::Server::HomeApplication homeApp;
    homeApp.setNotificationEnabled(notify);
    homeApp.setScreenConfiguration(fakeScreenData);
    if (parser.isSet(qmlOption)) {
        if (!homeApp.loadUrl(QUrl::fromLocalFile(parser.value(qmlOption))))
            return 1;
    } else {
        if (!homeApp.load(parser.value(shellOption)))
            return 1;
    }

    return app.exec();
}
