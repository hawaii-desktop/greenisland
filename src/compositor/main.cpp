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
#include <QtGui/QScreen>
#include <QtQml/QQmlApplicationEngine>

#include "compositor.h"
#include "config.h"
#include "logging.h"
#include "utilities.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

int main(int argc, char *argv[])
{
    // Application
    QGuiApplication app(argc, argv);
    app.setApplicationName("Green Island");
    app.setApplicationVersion(GREENISLAND_VERSION_STRING);
    app.setOrganizationDomain("maui-project.org");
    app.setOrganizationName("Maui");

    // Check whether XDG_RUNTIME_DIR is ok or not
    GreenIsland::verifyXdgRuntimeDir();

    // Enable debug messages
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("*.warning=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("*.critical=true"));

    // Set message pattern
    qSetMessagePattern("%{message}");

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("Command line parser", "Wayland compositor for the Hawaii desktop environment"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Wayland socket
    QCommandLineOption socketOption(QStringList() << QStringLiteral("s") << QStringLiteral("socket"),
                                    QCoreApplication::translate("Command line parser", "Wayland socket"),
                                    QCoreApplication::translate("Command line parser", "name"));
    parser.addOption(socketOption);

    // Synthesize touch for unhandled mouse events
    QCommandLineOption synthesizeOption(QStringLiteral("synthesize-touch"),
                                        QCoreApplication::translate("Command line parser", "Synthesize touch for unhandled mouse events"));
    parser.addOption(synthesizeOption);

    // Idle time
    QCommandLineOption idleTimeOption(QStringList() << QStringLiteral("i") << QStringLiteral("idle-time"),
                                      QCoreApplication::translate("Command line parser", "Idle time in seconds (at least 5 seconds)"),
                                      QCoreApplication::translate("Command line parser", "secs"));
    idleTimeOption.setDefaultValue("300");
    parser.addOption(idleTimeOption);

    // Fake screen configuration
    QCommandLineOption fakeScreenOption(QStringLiteral("fake-screen"),
                                         QCoreApplication::translate("Command line parser", "Use fake screen configuration"),
                                         QCoreApplication::translate("Command line parser", "filename"));
    parser.addOption(fakeScreenOption);

    // Parse command line
    parser.process(app);

    // If a socket is passed it means that we are nesting into
    // another compositor, let's do some checks
    const QString socket = parser.value(socketOption);
    if (!socket.isEmpty()) {
        // We need wayland QPA plugin
        if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            qWarning() << "By passing the \"--socket\" argument you are requesting to nest"
                       << "this compositor into another, but you forgot to pass "
                       << "also \"-platform wayland\"!";
#if HAVE_SYSTEMD
            sd_notifyf(0, "STATUS=Nesting requested, but no wayland QPA");
#endif
            return 1;
        }
    }

    // Set application attributes
    if (parser.isSet(synthesizeOption))
        app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    // Fake screen configuration
    if (parser.isSet(fakeScreenOption)) {
        // Use fake backend for KSCreen
        qputenv("KSCREEN_BACKEND", QByteArray("Fake"));
        qputenv("TEST_DATA", parser.value(fakeScreenOption).toUtf8());
    }

    // Create the compositor
    Compositor *compositor = new Compositor(socket);

    // Run the compositor QML code
    compositor->run();

    // Compositor options
    int idleInterval = parser.value(idleTimeOption).toInt();
    if (idleInterval >= 5)
        compositor->setIdleInterval(idleInterval * 1000);

#if HAVE_SYSTEMD
    sd_notifyf(0,
               "READY=1\n"
               "STATUS=Ready\n"
               "MAINPID=%llu",
               QCoreApplication::applicationPid());
#endif

    return app.exec();
}
